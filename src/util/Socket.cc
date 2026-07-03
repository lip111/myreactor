#include "Socket.h"
#include "unistd.h"
#include "sys/socket.h"
#include "netinet/tcp.h"
#include "fcntl.h"

namespace myreactor {

// 构造函数

Socket::Socket(int fd): fd_(fd) {}

Socket::~Socket() {
    if (fd_ >= 0)
        ::close(fd_);
}


// 查询

InetAddress Socket::getPeerAddr() const {
    
    sockaddr_in peerAddr;
    socklen_t n = sizeof(peerAddr);
    ::getpeername(fd_, (sockaddr*)&peerAddr, &n);
    return InetAddress(peerAddr);
} 

InetAddress Socket::getLocalAddr() const {

    sockaddr_in localAddr;
    socklen_t n = sizeof(localAddr);
    ::getsockname(fd_, (sockaddr*)&localAddr, &n);
    return InetAddress(localAddr);
}


// 设置
// 监听fd绑定服务器IP+端口
void Socket::bind(const InetAddress& addr) {
    ::bind(fd_, addr.getSockAddr(), addr.socklen());
}

// 未监听->监听, 创建连接队列
void Socket::listen(int backlog) {
    ::listen(fd_, backlog);
}

// 从已完成连接队列中取出一个连接, 分配一个connfd并返回, 对端ip+端口号 填入出参peerAddr
int Socket::accept(InetAddress* peerAddr) {

    sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int connfd = ::accept(fd_, (sockaddr*)&addr, &len);
    if (connfd >= 0 && peerAddr)
        peerAddr->setAddr(addr);

    return connfd;
}

// 用于客户端与服务器具体 ip+端口 建立连接
void Socket::connect(const InetAddress& serverAddr) {
    ::connect(fd_, serverAddr.getSockAddr(), serverAddr.socklen());
}

void Socket::setReuseAddr(bool on) {
    int optval = on ? 1: 0;
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}

void Socket::setTcpNoDelay(bool on) {
    int optval = on ? 1: 0;
    ::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}

void Socket::shutdownwrite() {
    ::shutdown(fd_, SHUT_WR);
}

void Socket::setNonBlocking(bool on) {
    int flags = ::fcntl(fd_, F_GETFL, 0);
    if (on)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;
    ::fcntl(fd_, F_SETFL, flags);
}

void Socket::setKeepAlive(bool on) {
    int optval = on ? 1: 0;
    ::setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)); 
}



}