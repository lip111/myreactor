#pragma once
#include <vector>
#include <unistd.h>

namespace myreactor {

class Buffer {
public:
    using size_type = std::size_t;
    Buffer();
    ~Buffer();

    // 由fd读取内核数据,存到buffer_
    ssize_t readFd(int fd, int* savedErrno);


    size_type readableBytes() const;
    size_type writableBytes() const;

    // 内核socket写入数据
    void append(const char* data, size_type n);
    // 标记消费字节数,起点是readerIdx_
    void retrieve(size_type n);
    // 可读数据开头位置指针
    const char* peek() const;
private:
    std::vector<char> buffer_;
    size_type readerIdx_;
    size_type writerIdx_;
};

}