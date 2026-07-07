#include "mytcpserver.h"
#include "mytcpconnection.h"
#include "myacceptor.h"

namespace myreactor {

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenaddr)
    :loop_(loop), localaddr_(listenaddr), acceptor_(std::make_unique<Acceptor>(loop, listenaddr)) {
        acceptor_->setNewConnectionCallback([this](int connfd, const InetAddress& peeraddr) {
            newConnection(connfd, peeraddr);
        });
}

TcpServer::~TcpServer() = default;

void TcpServer::newConnection(int connfd, const InetAddress& peeraddr) {

    auto conn = std::make_shared<TcpConnection>(loop_, connfd, localaddr_, peeraddr);
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

    // 触发连接建立回调
    if (connectionCallback_)
        connectionCallback_(conn);

}


void TcpServer::removeConnection(const std::shared_ptr<TcpConnection>& conn) {

    connections_.erase(conn->name());
    if (connectionCallback_)
        connectionCallback_(conn);
}


}