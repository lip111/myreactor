#include "myacceptor.h"
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <mylogger.h>
#include <cerrno>

namespace myreactor {

// 构造函数
Acceptor::Acceptor(EventLoop* loop, const InetAddress& addr)
    :loop_(loop),
    socket_(::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0)),
    channel_(loop, socket_.fd())
{
    socket_.setReuseAddr(true);
    socket_.bind(addr);  

    channel_.setReadCallback([this]() { handleRead(); });
    channel_.enableReading();
}

// channel负责取消注册fd,socket负责关闭fd！！！
Acceptor::~Acceptor() = default;

void Acceptor::listen() {
    socket_.listen();
}

void Acceptor::handleRead() {
    InetAddress peerAddr;
    int connfd = socket_.accept(&peerAddr);
    if (connfd < 0 && errno != EAGAIN)
        LOG_ERROR << "Accept failed, errno = " << errno;

    if (connfd >= 0) {
        LOG_INFO << "New connection from " << peerAddr.toIpPort() << ", fd = " << connfd;
        if (newConnectionCallback_)
            newConnectionCallback_(connfd, peerAddr);
        else
            ::close(connfd);
    }
    else {
        // todo 添加到日志，暂时先打印
        fprintf(stderr, "Acceptoror::handleRead Acceptor failed, errno=%d\n", errno);
    }
}

void Acceptor::setNewConnectionCallback(const NewConnectionCallback& cb) {
    newConnectionCallback_ = cb;
}

}