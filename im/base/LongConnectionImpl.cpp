#include "LinkBuffer.h"
#include <cstdint>
#include <iostream>
#include <memory>
#include "ILongConnection.h"
#include "im/base/Utility.h"

#include "LongConnectionImpl.h"

namespace roc::net {

LongConnectionImpl::LongConnectionImpl(boost::asio::io_context& io_context, const LongConnectionConfig& config)
    : io_context_(io_context),
      socket_(io_context),
      resolver_(io_context),
      reconnect_timer_(io_context),
      config_(config),
      write_buffer_(std::make_shared<base::LinkBuffer>()),
      read_buffer_(std::make_shared<base::LinkBuffer>()){}

void LongConnectionImpl::send_impl(const std::string& data) { 
       if (data.length() <= 0) {
            return;
        }
        write_buffer_->write(data.data(), data.length());
        do_write();
}

void LongConnectionImpl::send_impl(const char* data, size_t size) { 
    if (size < 0) {
         return;
     }
     std::cout<<"[call send_impl]"<<std::endl;

    char number_bytes[4];
    roc::base::util::encode_uint32_LE(size, number_bytes);

    std::cout<<"[write bytes]";
    for (int i = 0; i < 4; ++i) {
        std::cout<<uint8_t(number_bytes[i]);
    }
    std::cout<<std::endl;

    /// 分别写入长度 和 数据
    write_buffer_->write(number_bytes, 4);
    write_buffer_->write(data, size);
    do_write();
}

void LongConnectionImpl::send_impl(size_t size, SendDataBlockType&& sync_fill_data_block) {
    char number_bytes[4];
    roc::base::util::encode_uint32_LE((uint32_t)size, number_bytes);

    write_buffer_->write(number_bytes, 4);

    auto prepare_buffer = write_buffer_->prepare_buffers(size);
    roc::base::util::safe_invoke_block(sync_fill_data_block, prepare_buffer);
    write_buffer_->commit(size);

    do_write();
}

void LongConnectionImpl::connect_impl() {
    if (!connected_) {
        do_connect();
    }
}

void LongConnectionImpl::set_connect_callback_impl(ConnectCallback callback) {
    connect_callback_ = std::move(callback);
}

void LongConnectionImpl::set_receive_callback_impl(ReceiveCallback callback) {
    receive_callback_ = std::move(callback);
}

void LongConnectionImpl::disconnect_impl() {
    connected_ = false;
    boost::system::error_code ec;
    socket_.close(ec);
    reconnect_timer_.cancel();
}

void LongConnectionImpl::do_connect() {
    auto self = shared_from_this();
    resolver_.async_resolve(
        config_.host,
        config_.port,
        [self](const boost::system::error_code& ec, const auto& endpoints) {
            if (ec) {
                std::cout << "[resolve addr error]" << ec.message() << std::endl;
                self->schedule_reconnect();
                return;
            }

            boost::asio::async_connect(
                self->socket_,
                endpoints,
                [self](const boost::system::error_code& ec, const auto&) {
                    if (!ec) {
                        std::cout << "[connect success]" << std::endl;
                        self->connected_ = true;
                        if (self->connect_callback_) {
                            self->connect_callback_(self);
                        }
                        self->do_read();
                    } else {
                        std::cout << "[connect error]" << std::endl;
                        self->schedule_reconnect();
                    }
                });
        });
}

void LongConnectionImpl::do_read() {
    auto self = shared_from_this();
    socket_.async_read_some(
        self->read_buffer_->prepare_buffers(),
        [self](const boost::system::error_code& ec, size_t bytes_transferred) {
            if (ec) {
                self->handle_disconnect(ec);
                return;
            }

            /// 1、buffer 数据确认
            self->read_buffer_->commit(bytes_transferred);

            /// function 判断是否可以读取到整个包并处理
            auto handlePacktFunc = [self]() -> bool {
                if (!self->nextPackLen) {
                    return false;
                }

                uint32_t len = self->nextPackLen.value();
                if (self->read_buffer_->readable() >= len) {  /// 获取到一个完整包
                    auto buf = self->read_buffer_->get_read_buffers(len);
                    if (self->receive_callback_ && buf) {
                        self->receive_callback_(self, buf.value());
                    }
                    self->nextPackLen = std::nullopt;
                    return true;
                }

                return false;
            };

            /// 获取包长的function
            auto getPacketLenFunc = [self]() -> bool {
                if (self->nextPackLen) {
                    return true;
                }

                if (!self->nextPackLen && self->read_buffer_->readable() >= 4) {
                    auto buf = self->read_buffer_->get_read_buffers(4);
                    if (buf) {
                        self->nextPackLen = buf.value().peekUnint32();
                    }
                    buf->release();
                    return true;
                }
                return false;
            };

            bool loop = false;
            do {
                bool ok1 = getPacketLenFunc();

                if (self->nextPackLen) {
                    std::cout<<"[data len]"<<self->nextPackLen.value()<<std::endl;
                }

                bool ok2 = handlePacktFunc();
                loop = ok1 && ok2;
            } while(loop);

            self->do_read();

        });
}

void LongConnectionImpl::do_write() {
    auto self = shared_from_this();
    
    auto readResult = self->write_buffer_->get_read_buffers();
    if (!readResult){
        return;
    }

    auto str = readResult.value().readString();

    boost::asio::async_write(
        socket_,
        readResult.value().buffers,
        [self, readResult](const boost::system::error_code& ec, size_t) {
            if (ec) {
                self->handle_disconnect(ec);
                return;
            }
            
            std::cout<<"[write buffer]"<<self->write_buffer_<<std::endl;
            std::cout<<"[read buffer]"<<self->read_buffer_<<std::endl;
            readResult.value().release();

            if (self->write_buffer_->readable() > 0) {
                std::cout<<"read recall"<<std::endl;
                self->do_write();
            }
        });
}

void LongConnectionImpl::schedule_reconnect() {
    connected_ = false;
    reconnect_timer_.expires_after(
        std::chrono::milliseconds(config_.reconnect_interval));
    
    auto self = shared_from_this();
    reconnect_timer_.async_wait(
        [self](const boost::system::error_code& ec) {
            if (!ec) {
                self->do_connect();
            }
        });
}

void LongConnectionImpl::handle_disconnect(const boost::system::error_code& ec) {
    if (connected_) {
        connected_ = false;
        boost::system::error_code ignore_ec;
        socket_.close(ignore_ec);
        schedule_reconnect();
    }
}

} // namespace roc::net