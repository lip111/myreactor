#pragma once
#include "InetAddress.h"

namespace myreactor
{

class Socket {
public:
    // 构造函数
    explicit Socket(int fd);
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    ~Socket();

    // 查询
    int fd() const { return fd_; }
    InetAddress getPeerAddr() const;
    InetAddress getLocalAddr() const;

    // 设置
    void bind(const InetAddress& addr);
    void listen(int backlog = 128);
    int accept(InetAddress* peerAddr);
    void connect(const InetAddress& serverAddr);
    void close();

    void setReuseAddr(bool on);
    void setTcpNoDelay(bool on);
    void shutdownwrite();
    void setNonBlocking(bool on);
    void setKeepAlive(bool on);

private:
    int fd_;
};

}