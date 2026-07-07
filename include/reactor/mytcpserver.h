#pragma once
#include <memory>
#include <unordered_map>
#include <InetAddress.h>
#include <functional>

namespace myreactor {
class EventLoop;
class TcpConnection;
class Acceptor;
class Buffer;

class TcpServer {
public:
    using MessageCallback = std::function<void(const std::shared_ptr<TcpConnection>&, Buffer*)>;
    using ConnectionCallback = std::function<void(const std::shared_ptr<TcpConnection>&)>;

    TcpServer(EventLoop* loop, const InetAddress& listenaddr);
    ~TcpServer();

    void start();

    void setMessageCallback(MessageCallback cb) { messageCallback_ = std::move(cb); }
    void setConnectionCallback(ConnectionCallback cb) { connectionCallback_ = std::move(cb); }

private:
    void newConnection(int connfd, const InetAddress& peerAddr);
    void removeConnection(const std::shared_ptr<TcpConnection>& connection);


    EventLoop* loop_;
    InetAddress localaddr_;
    std::unique_ptr<Acceptor> acceptor_;
    std::unordered_map<std::string, std::shared_ptr<TcpConnection>> connections_;   // 保存活跃连接

    MessageCallback messageCallback_;
    ConnectionCallback connectionCallback_;
};

}