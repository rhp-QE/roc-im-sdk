#ifndef Base_config_h
#define Base_config_h

#include <boost/asio/io_context.hpp>

extern boost::asio::io_context net_io_context;
extern boost::asio::io_context main_io_context;
extern boost::asio::io_context sdk_io_context;


#endif