#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <boost/asio/io_context.hpp>
#include <memory>
#include <string>

namespace roc::imsdk {

struct Config {
public:
    int platform;
    std::string app_id;
    std::string app_key;
    std::string app_secret;
    std::string app_url;
    std::string app_port;
    std::string user_id;
    std::string user_token;
    std::string user_name;
    std::string user_avatar;
    std::string user_email;
    std::string user_phone;
    std::string user_address;
    std::string user_device_id;

    std::shared_ptr<boost::asio::io_context> net_io_context;
    std::shared_ptr<boost::asio::io_context> sdk_io_context;
    std::shared_ptr<boost::asio::io_context> db_io_context;

};

}

#endif // __CONFIG_H__