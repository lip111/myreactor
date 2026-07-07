#pragma once
#include "mychannel.h"
#include "Socket.h"
#include "InetAddress.h"


namespace myreactor {

class EventLoop;

class Acceptor {

public:
    using NewConnectionCallback = std::function<void(int connfd, const InetAddress& peeraddr)>;
    // 构造函数
    Acceptor(EventLoop* loop, const InetAddress& addr);
    ~Acceptor(); 

    // 设置
    void setNewConnectionCallback(const NewConnectionCallback& cb);

private:
    EventLoop* loop_;
    Socket acceptorSocket_;
    Channel acceptorChannel_;
    NewConnectionCallback newConnectionCallback_;

    void handleRead();
};

}