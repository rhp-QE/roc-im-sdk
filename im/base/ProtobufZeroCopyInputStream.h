//
// ProtobufZeroCopyInputStream.h
//
// author: Ruan Huipeng
// date : 2025-03-02
// 

#ifndef ROC_BASE_IO_PROTOBUFZEROCOPYINPUTSTREAM_H
#define ROC_BASE_IO_PROTOBUFZEROCOPYINPUTSTREAM_H

#include <boost/asio/buffer.hpp>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>


namespace roc::base::io {

class ProtobufZeroCopyInputStream : public google::protobuf::io::ZeroCopyInputStream {
public:
    explicit ProtobufZeroCopyInputStream(std::vector<boost::asio::const_buffer>& buffers) :
        buffers_(buffers),
        position_(0)
    {}

    bool Next(const void** data, int* size) override {
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

    bool Skip(int count) override{
        return false;
    }

private:
    int position_;
    std::vector<boost::asio::const_buffer>& buffers_;
};

} // namespace roc::base::io

#endif // ROC_BASE_IO_PROTOBUFZEROCOPYINPUTSTREAM_H
