#pragma once
#include "mychannel.h"
#include "Socket.h"
#include "InetAddress.h"


namespace myreactor {

class EventLoop;

class Acceptor {

public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress& addr)>;
    // 构造函数
    Acceptor(EventLoop* loop, const InetAddress& addr);
    ~Acceptor(); 

    // 设置
    void setNewConnectionCallback(const NewConnectionCallback& cb);

private:
    EventLoop* loop_;
    Socket AcceptorSocket_;
    Channel AcceptorChannel_;
    NewConnectionCallback newConnectionCallback_;

    void handleRead();
};

}