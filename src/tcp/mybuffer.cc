#include "mybuffer.h"
#include "sys/uio.h"
#include <cerrno>
#include <mylogger.h>

namespace myreactor {

Buffer::Buffer(): buffer_(1024),readerIdx_(0),writerIdx_(0) {}
Buffer::~Buffer() {}

typename Buffer::size_type Buffer::readableBytes() const {
    return writerIdx_ - readerIdx_;
}

typename Buffer::size_type Buffer::writableBytes() const {
    return buffer_.size() - writerIdx_;
}

void Buffer::append(const char* data, size_type n) {
    if (writableBytes() < n)
        buffer_.resize(writerIdx_+n);
    
    writerIdx_ += n;
}

void Buffer::retrieve(size_type n) {
    if (n < readableBytes())
        readerIdx_ += n;
    else {
        readerIdx_ = 0;
        writerIdx_ = 0;
    }
}

const char* Buffer::peek() const {
    return &*buffer_.begin() + readerIdx_;
}

ssize_t Buffer::readFd(int fd, int* savedErrno) {

    char buf[65536];
    iovec vec[2];
    vec[0].iov_base = &*buffer_.begin() + writerIdx_;
    vec[0].iov_len = writableBytes();
    vec[1].iov_base = buf;
    vec[1].iov_len = sizeof(buf);
    
    int cnt = vec[0].iov_len > vec[1].iov_len ? 1: 2;
    auto bytes = readv(fd, vec, cnt);
    if (bytes < 0) {
        *savedErrno = errno;
        LOG_ERROR << "Buffer::readFd failed, fd = " << fd << ", errno = " << errno; 
        return -1;
    }

    if (bytes <= vec[0].iov_len) {
        writerIdx_ += bytes;
    }
    else {
        writerIdx_ = buffer_.size();
        append(buf, bytes-vec[0].iov_len);
    }

    return bytes;
}



}