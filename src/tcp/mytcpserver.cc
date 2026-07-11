#include "mytcpserver.h"
#include "mytcpconnection.h"
#include "myacceptor.h"
#include "myeventloopthreadpool.h"
#include "myeventloop.h"
#include <iostream>

namespace myreactor {

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenaddr)
    :loop_(loop), localaddr_(listenaddr), 
    acceptor_(std::make_unique<Acceptor>(loop, listenaddr)),
    threadpool_(std::make_unique<EventLoopThreadPool>(loop_)) 
{
    acceptor_->setNewConnectionCallback([this](int connfd, const InetAddress& peeraddr) {
        newConnection(connfd, peeraddr);
    });
}

TcpServer::~TcpServer() = default;

void TcpServer::newConnection(int connfd, const InetAddress& peeraddr) {

    // auto conn = std::make_shared<TcpConnection>(loop_, connfd, localaddr_, peeraddr);
    // connections_.emplace(conn->name(), conn);
    EventLoop* slaveloop = threadpool_->getNextLoop(); 
    auto conn = std::make_shared<TcpConnection>(slaveloop, connfd, localaddr_, peeraddr);
    connections_.emplace(conn->name(), conn);

    // 设置读回调
    conn->setReadCallback([this](const std::shared_ptr<TcpConnection>& conn, Buffer* buffer){
        if (messageCallback_)
            messageCallback_(conn, buffer);
    });

    // 设置关闭回调
    conn->setCloseCallback([this](const std::shared_ptr<TcpConnection>& conn) {
        removeConnection(conn);
    });

    // // 触发连接建立回调
    // if (connectionCallback_)
    //     connectionCallback_(conn);

    // 连接注册到epoll实例，该任务投递到子线程执行
    slaveloop->queueInLoop([this, conn]() {
        conn->connectEstablished();
        if (connectionCallback_)
            connectionCallback_(conn);
    });

}

void TcpServer::removeConnection(const std::shared_ptr<TcpConnection>& conn) {

    loop_->queueInLoop([this, conn](){
        connections_.erase(conn->name());
        if (connectionCallback_)
            connectionCallback_(conn);
    });

}

void TcpServer::start() {
    std::cout << "TcpServer listening on " << localaddr_.toIpPort() << std::endl;
    threadpool_->start();
    loop_->runInLoop([this](){
        acceptor_->listen();
    });

}

void TcpServer::setThreadNum(int numThreads) {
    threadpool_->setThreadNum(numThreads);
}

}