//
// ProtobufZeroCopyOutStream.h
//
// author: Ruan Huipeng
// date : 2025-03-02
// 

#ifndef ROC_BASE_IO_PROTOBUFZEROCOPYOUTSTREAM_H
#define ROC_BASE_IO_PROTOBUFZEROCOPYOUTSTREAM_H

#include "google/protobuf/io/zero_copy_stream.h"
#include <boost/asio/buffer.hpp>


namespace roc::base::io {

class ProtobufZeroCopyOutStream : public google::protobuf::io::ZeroCopyOutputStream {
public:
    // 构造函数
    explicit ProtobufZeroCopyOutStream(std::vector<boost::asio::mutable_buffer>& buffers) :
        buffers_(buffers),
        position_(0)
    {}

    bool Next(void** data, int* size) override {
        if (position_ >= buffers_.size()) {
            return false;
        }
    
        *data = buffers_[position_].data();
        *size = buffers_[position_].size();
        position_ += 1;
        return true;
    }

    void BackUp(int count) override {
        
    }

  // Returns the total number of bytes written since this object was created.
    int64_t ByteCount() const override {
        return 1;
    }

private:
    int position_;
    std::vector<boost::asio::mutable_buffer>& buffers_;
};

} // namespace roc::base::io

#endif // ROC_BASE_IO_PROTOBUFZEROCOPYOUTSTREAM_H
