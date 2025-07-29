#include <cstddef>
#include <memory>
#include "ILongConnection.h"

#include "LinkBuffer.h"
#include <optional>

#ifndef LongConnection_H_
#define LongConnection_H_

namespace roc::net {

class LongConnectionImpl : public ILongConnection<LongConnectionImpl>, public std::enable_shared_from_this<LongConnectionImpl> {
public:
    LongConnectionImpl(boost::asio::io_context& io_context, const LongConnectionConfig& config);

    void send_impl(const std::string& data);
    void send_impl(const char* data, size_t size);
    void send_impl(size_t size, SendDataBlockType&& sync_fill_data_block);
    void connect_impl();
    void disconnect_impl();
    void set_connect_callback_impl(ConnectCallback callback);
    void set_receive_callback_impl(ReceiveCallback callback);

private:
    void do_connect();
    void do_read();
    void do_write();
    void schedule_reconnect();
    void handle_disconnect(const boost::system::error_code& ec);

    std::optional<uint32_t> nextPackLen = std::nullopt;

    boost::asio::io_context& io_context_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::ip::tcp::resolver resolver_;
    boost::asio::steady_timer reconnect_timer_;
    LongConnectionConfig config_;
    std::shared_ptr<roc::base::LinkBuffer> read_buffer_;
    std::shared_ptr<roc::base::LinkBuffer> write_buffer_;
    bool connected_ = false;

    std::shared_ptr<ILongConnection<LongConnectionImpl>> create_client(
        boost::asio::io_context& io_context,
        const LongConnectionConfig& config
    );

protected:
    ReceiveCallback receive_callback_;
    ConnectCallback connect_callback_;

};

} /// namespace roc::net

#endif