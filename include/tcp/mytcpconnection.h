#pragma once
#include "InetAddress.h"
#include "mychannel.h"
#include "Socket.h"
#include "mybuffer.h"
#include <memory>

namespace myreactor {

class EventLoop;
// 继承enable_shared_from_this防止一个对象创建多个共享指针，析构时double free
class TcpConnection: public std::enable_shared_from_this<TcpConnection> {
public:
    using ReadCallback = std::function<void(const std::shared_ptr<TcpConnection>& conn, Buffer*)>;
    using WriteCallback = std::function<void(const std::shared_ptr<TcpConnection>& conn)>;
    using CloseCallback = std::function<void(const std::shared_ptr<TcpConnection>& conn)>;
    using ErrorCallback = std::function<void(const std::shared_ptr<TcpConnection>& conn)>;
    using WriteCompleteCallback = std::function<void(const std::shared_ptr<TcpConnection>& conn)>;

    // 构造函数
    TcpConnection(EventLoop* loop, int connfd, const InetAddress& localaddr, const InetAddress& peeraddr);
    ~TcpConnection();

    void send(const std::string& message);
    EventLoop* getLoop() const;

    const InetAddress& localaddr() const { return localaddr_; }
    const InetAddress& peeraddr() const { return peeraddr_; }
    const std::string& name() { return name_; }
    bool connected() { return connected_; }
    void setConnected(bool flag) { connected_ = flag; }

    void setReadCallback(ReadCallback cb) {
        readCallback_ = std::move(cb);
    }

    // void setWriteCallback(WriteCallback cb) {
    //     writeCallback_ = std::move(cb);
    // }

    void setCloseCallback(CloseCallback cb) {
        closeCallback_ = std::move(cb);
    }

    void setErrorCallback(ErrorCallback cb) {
        errorCallback_ = std::move(cb);
    }

    void setWriteCompleteCallback(WriteCallback cb) {
        writeCompleteCallback_ = std::move(cb);
    }

    // 连接注册到epoll实例，使能可读事件
    void connectEstablished();

    // 设置空闲超时
    void enableIdleTimeout(int seconds);
    void setIdleTimeout(int seconds);

private:
    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();


    EventLoop* loop_;
    InetAddress localaddr_;
    InetAddress peeraddr_;
    Socket socket_;
    Channel channel_;
    std::string name_;
    bool connected_;    // 标记连接就绪,可收发数据

    ReadCallback readCallback_; // 业务相关,从内核读取数据,需要往上传递给tcpserver,tcpserver再提供给用户，最终由用户处理！！！
    // WriteCallback writeCallback_;   业务无关,不需要tcpserver参与！！！
    CloseCallback closeCallback_;
    ErrorCallback errorCallback_;
    WriteCompleteCallback writeCompleteCallback_;   // 数据写完这个事情很重要，需要通知tcpserver！！！

    Buffer inputbuffer_;
    Buffer outputbuffer_;

    int idleSeconds_ = 0;   // 0表示不启用空闲超时
    int idleTimerId_ = -1;   // 空闲超时定时器id,-1表示未设置
};


}