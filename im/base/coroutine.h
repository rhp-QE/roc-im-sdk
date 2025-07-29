#ifndef ROC_COROUTINE_H
#define ROC_COROUTINE_H


#include <algorithm>
#include <boost/core/noncopyable.hpp>
#include <coroutine>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <im/base/noncopyable.h>
#include <utility>
#include <variant>
#include <vector>

namespace roc::coro {

// ------------------- pre declare (begin) --------------------

template<typename... ReType>
class AwaitableAll;

// ------------------- pre declare (end)   --------------------

// ===================== std:coroutine RAII =====================
class CoroRAII {
    using handle_type = std::coroutine_handle<>;
public:
    CoroRAII(handle_type coro_handle) : coro_handle_(coro_handle) {}
    ~CoroRAII() {
        if (coro_handle_) { coro_handle_.destroy(); }
    }

    handle_type coro_handle() { return coro_handle_; }
private: 
    handle_type coro_handle_;
};

//------------------------ std::coroutine RAII ---------------------

//-------------------   协程生命周期管理  -----------------------------
extern std::mutex coro_mutex;
extern std::unordered_map<size_t, std::shared_ptr<CoroRAII>> coro_manager;
inline size_t ref_coro(std::shared_ptr<CoroRAII> coro) {
    std::lock_guard<std::mutex> lock(coro_mutex);
    static size_t id = 0;
    ++id;
    coro_manager[id] = coro;
    return id; 
}

inline void unref_coro(size_t id) {
    std::lock_guard<std::mutex> lock(coro_mutex);
    coro_manager.erase(id);
}
//-------------------------------------------------------------------


// ==================  co_async 封装  (存在返回值)   ===================
template<typename Type = void>
struct co_async {

    struct promise_type;
    struct awaiter;

    std::shared_ptr<CoroRAII> coro_arii_sptr;
    std::coroutine_handle<promise_type> handle;

    using ReType = Type;

    co_async(std::shared_ptr<CoroRAII> h) : coro_arii_sptr(h) { 
        handle =  std::coroutine_handle<promise_type>::from_address(coro_arii_sptr->coro_handle().address());
    }

    auto operator co_await() noexcept {
        return awaiter{ 
            std::coroutine_handle<promise_type>::from_address(coro_arii_sptr->coro_handle().address())
        };
    }

    // =================== awaiter ================
    struct awaiter {
        std::coroutine_handle<promise_type>  coro_handle;

        bool await_ready() const noexcept {
            bool ready = false;
            {
                std::lock_guard<std::mutex> lock(coro_handle.promise().mutex);
                ready = !coro_handle || (coro_handle.promise().result != std::nullopt);
            } // promise lock
            return ready;
        }

        void await_suspend(std::coroutine_handle<> awaiting_coro) noexcept {
            std::lock_guard<std::mutex> lock(coro_handle.promise().mutex);
            coro_handle.promise().parent = awaiting_coro;
        } // promise lock

        ReType await_resume() {
            ReType res;
            {
                std::lock_guard<std::mutex> lock(coro_handle.promise().mutex);
                res = coro_handle.promise().result.value();
            } // promise lock
            return res;
        }
    };
    // ----------------- awaiter --------------------

    // ---------------  promise_type (being) -------------------
    
    // promise_type 会暴露给多个线程访
    struct promise_type {
    
        size_t id;

        std::mutex mutex;
        bool completed;
        std::optional<ReType>   result;    // 协程的返回值
        std::coroutine_handle<> parent;    // 保存父协程句柄
        std::function<void(ReType retule)> callback;  // 当前协程任务完成回调

        co_async get_return_object() {
            auto h = std::coroutine_handle<promise_type>::from_promise(*this);
            auto handle =  std::make_shared<CoroRAII>(std::coroutine_handle<>::from_address(h.address()));
            id = ref_coro(handle);  // 引用计数加一

            completed = false;
            result = std::nullopt;

            return co_async{ handle };
        }

        std::suspend_never initial_suspend() noexcept { return {}; }

        auto final_suspend() noexcept {
            struct FinalAwaiter {
                bool await_ready() noexcept {  return false; }
                void await_suspend(std::coroutine_handle<promise_type> self) noexcept {
                    auto& promise = self.promise();
                    
                    std::coroutine_handle<> parent;
                    std::function<void(ReType result)> callback;
                    {
                        std::lock_guard<std::mutex> lock(promise.mutex);
                        parent = promise.parent;
                        callback = promise.callback;
                    } // promise lock

                    if (parent) { // 恢复父协程
                       parent.resume();
                    }

                    if (callback) { // 向外通知协程任务完成
                       callback(promise.result.value());
                    }

                    unref_coro(promise.id);
                }
                void await_resume() noexcept { }
            };
            return FinalAwaiter{};
        }

        void return_value(ReType v) noexcept {
            std::lock_guard<std::mutex> lock(mutex); 
            result = std::move(v);
            completed = true;
        } // lock promsie

        void unhandled_exception() { std::terminate(); }
    };
    // ------------------------ promise_type (end) --------------------

};
// ------------------------ co_async 封装 ------------------------


// ==================  co_async 封装  (返回值为空)   ==============
template<>
struct co_async<void> {

    struct promise_type;
    struct awaiter;

    std::shared_ptr<CoroRAII> coro_arii_sptr;
    std::coroutine_handle<promise_type> handle;

    co_async(std::shared_ptr<CoroRAII> h) : coro_arii_sptr(h) { 
        handle = std::coroutine_handle<promise_type>::from_address(coro_arii_sptr->coro_handle().address());
    }

    auto operator co_await() noexcept {
        return awaiter{ 
            std::coroutine_handle<promise_type>::from_address(coro_arii_sptr->coro_handle().address())
        };
    }

    void via() {
        handle.resume();
    }

    // =================== awaiter ================
    struct awaiter {
        std::coroutine_handle<promise_type> coro_handle;

        bool await_ready() const noexcept {
            return !coro_handle || coro_handle.done();
        }

        void await_suspend(std::coroutine_handle<> awaiting_coro) noexcept {
            coro_handle.promise().parent = awaiting_coro;
        }

        void await_resume() {}
    };
    // ----------------- awaiter --------------------

    // ================  promise_type   =============
    struct promise_type {
    
        size_t id;
        bool completed;
        std::mutex mutex;
        std::coroutine_handle<> parent; // 保存父协程句柄
        std::function<void()> callback;

        co_async get_return_object() {
            auto h = std::coroutine_handle<promise_type>::from_promise(*this);
            auto handle =  std::make_shared<CoroRAII>(std::coroutine_handle<>::from_address(h.address()));
            id = ref_coro(handle);

            return co_async{ handle };
        }

        std::suspend_never initial_suspend() noexcept { return {}; }

        auto final_suspend() noexcept {
            struct FinalAwaiter {
                bool await_ready() noexcept {  return false; }
                void await_suspend(std::coroutine_handle<promise_type> self) noexcept {
                    auto& promise = self.promise();
                    unref_coro(self.promise().id);

                    std::coroutine_handle<> parent;
                    std::function<void(void)> callback;
                    {
                        std::lock_guard<std::mutex> lock(promise.mutex);
                        parent = promise.parent;
                        callback = promise.callback;
                    } // promise lock

                    if (parent) { // 恢复父协程
                       parent.resume();
                    }

                    if (callback) { // 向外通知协程任务完成
                       callback();
                    }
                }
                void await_resume() noexcept {}
            };
            return FinalAwaiter{};
        }

        void return_void() {
            std::lock_guard<std::mutex> lock(mutex);
            completed = true;
        } // promise lock

        void unhandled_exception() { std::terminate(); }
    };
    // ------------------------ promise_type --------------------

};
// ------------------------ co_async 封装 ------------------------



// ===== 封装 awaitable_wapper (便于将异步回调函数封装成awaitable) =====
template <typename Ty = void>
struct co_awaitable_wapper;

template <typename Ty = void>
class CoroPromise;

// -------- coroutine promise ------------
template <typename Ty>
class CoroPromise{
    friend co_awaitable_wapper<Ty>;

public:
    void set_value(const Ty& value) {
        promise_->set_value(value);
        h.resume();
    }

private:
    std::shared_ptr<std::promise<Ty>> promise_;
    std::coroutine_handle<> h;
};

// 偏特化版本
// -------- coroutine promise ------------
template <>
class CoroPromise<void>{
    friend co_awaitable_wapper<void>;

public:
    void set_value() {
        promise_->set_value();
        h.resume();
    }

private:
    std::shared_ptr<std::promise<void>> promise_;
    std::coroutine_handle<> h;
};


template <typename ReType>
class co_awaitable_wapper {

    using Type = std::function<void(CoroPromise<ReType> promise)>;

    std::future<ReType> future;
    Type func;

public:
    bool await_ready() const noexcept { return false; }

    void await_suspend(std::coroutine_handle<> h) {
        CoroPromise<ReType> co_promise;
        co_promise.h = h;
        co_promise.promise_ = std::make_shared<std::promise<ReType>>();
        future = co_promise.promise_->get_future();

        func(CoroPromise<ReType>(std::move(co_promise)));
    }

    ReType await_resume() noexcept { return future.get(); }

    co_awaitable_wapper(Type fun) : func(fun) {}
};

// --------------------- 封装 await_able_wapper ---------------------

// --------------------- 封装 await all (begin) ---------------------

// 将 void 替换为 std::monostate（或其他占位类型）
template <typename T>
struct ReplaceVoid {
    using type = T;
};

template <>
struct ReplaceVoid<void> {
    using type = std::monostate;  // 替换为 std::monostate
};

template<typename... ReType>
class AwaitableAll {
public:
    std::tuple<co_async<ReType>...> awaitCoro_;
    std::tuple<typename ReplaceVoid<ReType>::type...> result;
    std::mutex mutex;
    int cnt_;
    std::coroutine_handle<> parent;

    template<size_t index>
    void register_callback() {
        auto& co = std::get<index>(awaitCoro_);
        using Type = std::tuple_element_t<index, std::tuple<ReType...>>;

        std::lock_guard<std::mutex> lock(co.handle.promise().mutex);

        if constexpr (std::is_void_v<Type>) {
            auto func = [this]() {
                std::lock_guard<std::mutex> lock(mutex);
                if (--cnt_ == 0 && parent) {
                    parent.resume();
                }
            }; // awaitable lock

            if (co.handle.promise().completed) {
                func();
            } else {
                co.handle.promise().callback = func;
            }
        } else {
            auto func = [this](Type res) {
                std::lock_guard<std::mutex> lock(mutex);
                std::get<index>(result) = std::move(res);
                if (--cnt_ == 0 && parent) {
                    parent.resume();
                }
            }; // awaiable lock

            if (co.handle.promise().completed) {
                func(co.handle.promise().result.value());
            } else {
                co.handle.promise().callback = func;
            }
        }
    } // promise lock
    
    AwaitableAll(std::tuple<co_async<ReType>...> awaitCoro, int cnt) : 
        awaitCoro_(awaitCoro),
        cnt_(cnt)
    {
        auto index = std::make_index_sequence<std::tuple_size_v<decltype(awaitCoro_)>>();

        [&]<size_t... I>(std::index_sequence<I...>) {
            (..., register_callback<I>());
        }(index);
    }


    bool await_ready() {
        bool ok = false;
        {
            std::lock_guard<std::mutex> lock(mutex);
            ok = (cnt_ <= 0);
        } // awaiate lock
        return ok;
    }

    void await_suspend(std::coroutine_handle<> h) {
        parent = h;
    }

    std::tuple<typename ReplaceVoid<ReType>::type ...> await_resume() noexcept { return result; }
};


template <typename... ReType>
constexpr AwaitableAll<ReType...> when_all(std::tuple<co_async<ReType>...> cos) {
    return AwaitableAll<ReType...>(cos, int(std::tuple_size_v<decltype(cos)>));
}


template <typename... ReType>
constexpr AwaitableAll<ReType...> when_any(std::tuple<co_async<ReType>...> cos) {
    return AwaitableAll<ReType...>(cos, 1);
}

// --------------------- 封装 await all  (end) ---------------------


} // namespace roc::coro

#endif