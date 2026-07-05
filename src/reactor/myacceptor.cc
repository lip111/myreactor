#include "myacceptor.h"
#include <sys/socket.h>
#include <unistd.h>

namespace myreactor {

// 构造函数
Acceptor::Acceptor(EventLoop* loop, const InetAddress& addr)
    :loop_(loop),AcceptorSocket_(::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0)),
    AcceptorChannel_(loop,AcceptorSocket_.fd())
    {
        AcceptorSocket_.setReuseAddr(true);
        AcceptorSocket_.bind(addr);
        AcceptorSocket_.listen();

        AcceptorChannel_.setReadCallback([this]() { handleRead(); });
        AcceptorChannel_.enableReading();
    }
Acceptor::~Acceptor() {}


void Acceptor::handleRead() {
    InetAddress peerAddr;
    int connfd = AcceptorSocket_.accept(&peerAddr);
    if (connfd >= 0) {
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