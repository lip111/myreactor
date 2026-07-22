#include "mytcpserver.h"
#include "mytcpconnection.h"
#include "myacceptor.h"
#include "myeventloopthreadpool.h"
#include "myeventloop.h"
#include <iostream>
#include <mylogger.h>

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
    EventLoop* subloop = threadpool_->getNextLoop(); 
    auto conn = std::make_shared<TcpConnection>(subloop, connfd, localaddr_, peeraddr);
    connections_.emplace(conn->name(), conn);
    LOG_INFO << "Dispatch Tcpconnection from " << peeraddr.toIpPort() << " to thread " << subloop->threadId();

    // 设置读回调
    conn->setReadCallback([this](const std::shared_ptr<TcpConnection>& conn, Buffer* buffer){
        if (messageCallback_)
            messageCallback_(conn, buffer);
    });

    // 设置关闭回调
    conn->setCloseCallback([this](const std::shared_ptr<TcpConnection>& conn) {
        removeConnection(conn);
    });

    // 设置空闲超时,单位/秒
    conn->setIdleTimeout(30);

    // // 触发连接建立回调
    // if (connectionCallback_)
    //     connectionCallback_(conn);

    // 连接注册到epoll实例，该任务投递到子线程执行
    subloop->queueInLoop([this, conn]() {
        conn->connectEstablished();
        if (connectionCallback_)
            connectionCallback_(conn);
    });

}

void TcpServer::removeConnection(const std::shared_ptr<TcpConnection>& conn) {

    loop_->queueInLoop([this, conn](){
        auto deleted = connections_.erase(conn->name());
        if (deleted > 0)
            LOG_INFO << "Remove Tcpconnction from " << conn->peeraddr().toIpPort() << ", remaining: " << connections_.size(); 
        if (connectionCallback_)
            connectionCallback_(conn);
    });

}

void TcpServer::start() {
    threadpool_->start();
    loop_->runInLoop([this](){
        acceptor_->listen();
    });
    LOG_INFO << "TcpServer started on " << localaddr_.toIpPort() << " with " << threadpool_->numThreads() << " threads";

}

void TcpServer::setThreadNum(int numThreads) {
    threadpool_->setThreadNum(numThreads);
}

}