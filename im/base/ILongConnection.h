#ifndef NET_CLIENT_H
#define NET_CLIENT_H

#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <cstddef>
#include <functional>
#include <linux/stat.h>
#include <memory>
#include <vector>
#include "LinkBuffer.h"


namespace roc::net {


template<typename Ty>
class ILongConnection;

struct LongConnectionConfig {
    std::string host;
    std::string port;
    uint32_t reconnect_interval = 5000; // 重连间隔，单位毫秒
};

// using ReceiveCallback = std::function<void(std::shared_ptr<ILongConnection<LongConnectionImpl>> con, base::LinkBuffer::ReadResult readResult)>;
// using ConnectCallback = std::function<void(std::shared_ptr<ILongConnection<LongConnectionImpl>> con)>;

// using SendDataBlockType = std::function<void(std::vector<boost::asio::mutable_buffer>&)>;

///----------------- Interface ---------------------------
template<typename T>
class ILongConnection {
public:

    using ReceiveCallback = std::function<void(std::shared_ptr<ILongConnection<T>> con, base::LinkBuffer::ReadResult readResult)>;
    using ConnectCallback = std::function<void(std::shared_ptr<ILongConnection<T>> con)>;
    using SendDataBlockType = std::function<void(std::vector<boost::asio::mutable_buffer>&)>;

    virtual ~ILongConnection() = default;
    void set_receive_callback(ReceiveCallback callback);
    void set_connect_callback(ConnectCallback callback);
    void send(const std::string& data);
    void send(const char* data, size_t size);
    void send(size_t size, SendDataBlockType&& sync_fill_data_block);
    void connect();
    void disconnect();
};
/// -----------------------------------------------------


template<typename T>
void ILongConnection<T>::set_receive_callback(ReceiveCallback callback) {
    static_cast<T*>(this)->set_receive_callback_impl(callback);
}

template<typename T>
void ILongConnection<T>::set_connect_callback(ConnectCallback callback) {
    static_cast<T*>(this)->set_connect_callback_impl(callback);
}

template<typename T>
void ILongConnection<T>::send(const std::string& data) {
    static_cast<T*>(this)->send_impl(data);
}

template<typename T>
void ILongConnection<T>::send(const char* data, size_t size) {
    static_cast<T*>(this)->send_impl(data, size);
}

template<typename T>
void ILongConnection<T>::send(size_t size, SendDataBlockType&& sync_fill_data_block) {
    static_cast<T*>(this)->send_impl(size, std::move(sync_fill_data_block));
}

template<typename T>
void ILongConnection<T>::connect() {
    static_cast<T*>(this)->connect_impl();
}

template<typename T>
void ILongConnection<T>::disconnect() {
    static_cast<T*>(this)->disconnect_impl();
}

} // namespace roc::net

#endif // NET_CLIENT_H