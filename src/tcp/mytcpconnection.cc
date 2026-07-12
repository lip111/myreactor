#include "mytcpconnection.h"
#include <cstring>
#include <iostream>
#include <thread>
#include "mylogger.h"
#include <cerrno>

namespace myreactor {

TcpConnection::TcpConnection(EventLoop* loop, int connfd, const InetAddress& localaddr, const InetAddress& peeraddr) 
    :loop_(loop),  
    localaddr_(localaddr), peeraddr_(peeraddr), 
    socket_(connfd),
    channel_(loop, connfd),
    name_(peeraddr.toIpPort()), connected_(false) 
{        
    socket_.setNonBlocking(true);
    channel_.setReadCallback([this]() { handleRead(); });
    channel_.setWriteCallback([this]() { handleWrite(); });
    // channel_.enableReading();    
}

// channel负责取消注册fd
TcpConnection::~TcpConnection() = default;

EventLoop* TcpConnection::getLoop() const {
    return loop_;
}

// 从socket读取数据，放入输入缓冲区，通知上层处理
void TcpConnection::handleRead() {

    int savedErrno = 0;
    auto n = inputbuffer_.readFd(socket_.fd(), &savedErrno);
    // inputbuffer_.readFd(socket_.fd(), &savedErrno);   //  测试一下通信socket阻塞模式下的线程阻塞！！！
    if (n > 0) {
        LOG_INFO << "receive " << n << " bytes, msg: " << inputbuffer_.peek();
        if (readCallback_) {
            // 共享指针延长TcpConnection对象生命周期,避免异步执行时对象销毁
            readCallback_(shared_from_this(), &inputbuffer_);
        }
    }
    else if (n == 0) {   // 对端关闭连接
        handleClose();
    }
    else {   // 出错
        errno = savedErrno;
        LOG_ERROR << "Tcpconnection read failed: fd = " << socket_.fd() << ", errno = " << errno;
        handleError();
    }
}

// 往socket写入数据，通知上层处理
void TcpConnection::handleWrite() {
    auto n = ::write(socket_.fd(), outputbuffer_.peek(), outputbuffer_.readableBytes());
    if (n > 0) {
        LOG_DEBUG << "Send " << n << "bytes to fd = " << socket_.fd();
        outputbuffer_.retrieve(n);
        if (0 == outputbuffer_.readableBytes()) {
            if (writeCompleteCallback_)
                writeCompleteCallback_(shared_from_this());
        }
    }
    else if (n < 0) {
        if (errno == EPIPE || errno == ECONNRESET) {  // 对端连接关闭(EPIPE正常关闭，ECONNRESET异常关闭)
            LOG_WARN << "write to closed peer, fd = " << socket_.fd();
            handleClose();
        }
        else if (errno != EAGAIN) {
            LOG_ERROR << "Tcpconnection write failed: fd = " << socket_.fd() << ", errno = " << errno;
            handleError();
        }

    }
}

// 将注册的fd从epoll中删除,通知上层处理
void TcpConnection::handleClose() {
    LOG_INFO << "Connection closed, fd = " << socket_.fd();
    channel_.remove();
    socket_.close();
    setConnected(false);    // 关闭之前设置下状态
    if (closeCallback_)
        closeCallback_(shared_from_this());
}

void TcpConnection::handleError() {
    handleClose();
}

void TcpConnection::send(const std::string& message) {

    if (message.empty()) return;

    int message_len = message.size();
    const char* data = message.data();

    if (!channel_.isWriting()) {
        auto n = ::write(socket_.fd(), data, message_len);
        if (n == message_len)
            return;
        else if (n >= 0 && n < message_len) {
            data += n;
            message_len -= n;
        }
        else if (errno != EAGAIN) {
            handleError();
            return;
        }
    }

    // 当正在处理写事件或者直接send没发送完成或者错误码是EAGAIN
    outputbuffer_.append(data, message_len);
    if (!channel_.isWriting()) {
        channel_.enableWriting();
    } 
}

void TcpConnection::connectEstablished() {
    connected_ = true;
    channel_.enableReading();
    LOG_INFO << "Connection established, fd = " << socket_.fd();
}

}