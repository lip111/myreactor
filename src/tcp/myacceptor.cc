#include "myacceptor.h"
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

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
    // int flags = fcntl(connfd, F_GETFL, 0);
    // fcntl(connfd, F_SETFL, flags | O_NONBLOCK); // 通信socket一定要设置为非阻塞！！！
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