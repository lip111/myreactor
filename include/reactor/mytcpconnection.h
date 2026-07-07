#pragma once
#include "InetAddress.h"
#include "mychannel.h"
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


    const InetAddress& localaddr() const { return localaddr_; }
    const InetAddress& peeraddr() const { return peeraddr_; }
    const std::string& name() { return name_; }
    bool connected() { return connected_; }
    bool setConnected(bool flag) { connected_ = flag; }

    void setReadCallback(ReadCallback cb) {
        readCallback_ = std::move(cb);
    }

    void setWriteCallback(WriteCallback cb) {
        writeCallback_ = std::move(cb);
    }

    void setCloseCallback(CloseCallback cb) {
        closeCallback_ = std::move(cb);
    }

    void setErrorCallback(ErrorCallback cb) {
        errorCallback_ = std::move(cb);
    }

    void setWriteCompleteCallback(WriteCallback cb) {
        writeCallback_ = std::move(cb);
    }
private:
    EventLoop* loop_;
    int connfd_;
    InetAddress localaddr_;
    InetAddress peeraddr_;
    Channel connChannel_;
    std::string name_;
    bool connected_;

    ReadCallback readCallback_;
    WriteCallback writeCallback_;
    CloseCallback closeCallback_;
    ErrorCallback errorCallback_;
    WriteCompleteCallback writeCompleteCallback_;

    Buffer inputbuffer_;
    Buffer outputbuffer_;


    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();
};


}