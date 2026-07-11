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

    // 不在构造函数中开启监听,让tcpserver控制监听开始时机！！！
    void listen();

private:
    void handleRead();

    EventLoop* loop_;
    Socket socket_;
    Channel channel_;
    NewConnectionCallback newConnectionCallback_;
};

}