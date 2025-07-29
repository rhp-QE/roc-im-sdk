#include "BaseConfig.h"
#include <boost/asio/awaitable.hpp>
#include <cstdio>
#include <iostream>
#include <memory>
#include "base/network/include/LongConnectionClient.h"
#include "base/network/examples/simple_http_example.h"

boost::asio::awaitable<void> wsclientTest() {
    using ws_type = roc::base::net::WSClient;
    std::shared_ptr<roc::base::net::IWSClient<ws_type>> lc = std::make_shared<ws_type>(roc::base::net::WSClientConfig{"127.0.0.1", "10010"}, net_io_context);
    
    co_await lc->connect();
    std::string msg = "Hello, WebSocket!";
    auto res = co_await lc->send(msg.data(), msg.size());
    if (res.has_value()) {
        std::cout << "Sent " << res.value() << " bytes." << std::endl;
    } else {
        std::cerr << "Failed to send message: " << res.error() << std::endl;
    }

    while (true) {
        auto res_read = co_await lc->read();
        auto res_read1 = co_await lc->read();
        printf("received a message\n");
    }
    

    co_await lc->close();
    co_return;
}


inline boost::asio::awaitable<void> lonclientTest() {
    roc::base::net::LongConnectionConfig config("127.0.0.1", "10010");
    config.set_heartbeat_interval(30000);
    config.set_heartbeat_timeout(10000);
    config.set_heartbeat_payload("ping");
    config.set_auto_reconnect(true);
    config.set_max_reconnect_attempts(5);
    config.set_reconnect_backoff(1000);
    config.add_header("User-Agent", "LongConnectionClient/1.0");
    config.add_query_param("sendID", "RhpUserID");
    config.add_query_param("sdkType", "rocSDK-c++");

    auto lc = std::make_shared<roc::base::net::LongConnectionClient>(config, net_io_context);

    lc->set_data_received_callback([](boost::beast::flat_buffer) {
        std::cout << "Received data: "<<std::endl;
    });

    lc->set_connection_status_callback([](bool connected, const std::string& reason) {
        std::cout << "Connection status changed: " << (connected ? "Connected" : "Disconnected") << " - " << reason << std::endl;
    });

    co_await lc->connect();

    while(true)  {
        std::string msg = "Hello, WebSocket!";
        std::expected<size_t, roc::error::Error> res = co_await lc->send_data(msg.data(), msg.size());

        if (res.has_value()) {
            std::cout << "Sent " << res.value() << " bytes." << std::endl;
        } else {
            std::cerr << "Failed to send message: " << res.error().to_string() << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    co_return;
}


void wsclientTestMain() {
    boost::asio::co_spawn(net_io_context, lonclientTest(), boost::asio::detached);
    // boost::asio::co_spawn(net_io_context, http_examples(net_io_context), boost::asio::detached);
}

