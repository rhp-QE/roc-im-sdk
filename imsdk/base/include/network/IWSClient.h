//
// IWSClient.h
//
// author: Ruan Huipeng
// date : 2025-03-23
// 

#ifndef ROC_NET_IWSCLIENT_H
#define ROC_NET_IWSCLIENT_H

#include <boost/asio/awaitable.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <expected>
#include "imsdk/base/include/network/Error.h"

namespace roc::base::net {

template<typename T>
class IWSClient {
public:

    boost::asio::awaitable<std::expected<bool, roc::error::Error>> connect();
    boost::asio::awaitable<std::expected<bool, roc::error::Error>> close();
    boost::asio::awaitable<std::expected<size_t, roc::error::Error>> send(void *data, size_t size); 
    boost::asio::awaitable<boost::beast::flat_buffer> read();
    
    boost::asio::awaitable<std::expected<bool, roc::error::Error>> ping(const std::string& payload = "");
    boost::asio::awaitable<std::expected<bool, roc::error::Error>> pong(const std::string& payload = "");
    bool is_connected();

};

// -------------- impl --------------

template <typename T>
boost::asio::awaitable<std::expected<bool, roc::error::Error>> IWSClient<T>::connect() {
   return static_cast<T*>(this) -> connect_impl();
}

template <typename T>
boost::asio::awaitable<std::expected<bool, roc::error::Error>> IWSClient<T>::close() {
    return static_cast<T*>(this) -> close_impl();
}

template <typename T>
boost::asio::awaitable<std::expected<size_t, roc::error::Error>> IWSClient<T>::send(void *data, size_t size){
    return static_cast<T*>(this) -> send_impl(data, size);
}

template <typename T>
boost::asio::awaitable<boost::beast::flat_buffer> IWSClient<T>::read() {
    return static_cast<T*>(this) -> read_impl();
}

template <typename T>
boost::asio::awaitable<std::expected<bool, roc::error::Error>> IWSClient<T>::ping(const std::string& payload) {
    return static_cast<T*>(this) -> ping_impl(payload);
}

template <typename T>
boost::asio::awaitable<std::expected<bool, roc::error::Error>> IWSClient<T>::pong(const std::string& payload) {
    return static_cast<T*>(this) -> pong_impl(payload);
}

template<typename T>
bool IWSClient<T>::is_connected() {
    return static_cast<T*>(this) -> is_connected_impl();
}

} // namespace roc::base::net

#endif // ROC_NET_IWSCLIENT_H

