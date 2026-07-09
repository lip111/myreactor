#include "mytcpconnection.h"
#include <cstring>

namespace myreactor {

TcpConnection::TcpConnection(EventLoop* loop, int connfd, const InetAddress& localaddr, const InetAddress& peeraddr) 
    :loop_(loop), connfd_(connfd), localaddr_(localaddr), peeraddr_(peeraddr), connChannel_(loop, connfd),
    name_(peeraddr.toIpPort()), connected_(true) {
        
        connChannel_.setReadCallback([this]() { handleRead(); });
        connChannel_.setWriteCallback([this]() { handleWrite(); });
        connChannel_.enableReading();
    }

TcpConnection::~TcpConnection() {
    ::close(connfd_);
}

// 从socket读取数据，放入输入缓冲区，通知上层处理
void TcpConnection::handleRead() {

    int savedErrno = 0;
    auto n = inputbuffer_.readFd(connfd_, &savedErrno);
    // inputbuffer_.readFd(connfd_, &savedErrno);   //  测试一下通信socket阻塞模式下的线程阻塞！！！
    if (n > 0) {
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
        handleError();
    }
}

// 往socket写入数据，通知上层处理
void TcpConnection::handleWrite() {
    auto n = ::write(connfd_, outputbuffer_.peek(), outputbuffer_.readableBytes());
    if (n > 0) {
        outputbuffer_.retrieve(n);
        if (0 == outputbuffer_.readableBytes()) {
            if (writeCompleteCallback_)
                writeCompleteCallback_(shared_from_this());
        }
    }
    else if (n == 0) {   // 对端关闭连接
        handleClose();
    }
    else if (n == -1 && errno != EAGAIN) {   // 返回-1且不是EAGAIN才是真正出错
        handleError();
    }
}

// 将注册的fd从epoll中删除,通知上层处理
void TcpConnection::handleClose() {
    connChannel_.remove();
    ::close(connfd_);
    setConnected(false);    // 关闭之前设置下状态
    if (closeCallback_)
        closeCallback_(shared_from_this());
}

void TcpConnection::handleError() {
    int err = errno;
    
    // todo 添加到日志，暂时先打印
    fprintf(stderr, "TcpConnection::handleError [%s:%d] error: %d (%s)\n",
        peeraddr_.toIp().c_str(), peeraddr_.toPort(), err, strerror(err));
    
    handleClose();
}

void TcpConnection::send(const std::string& message) {

    if (message.empty()) return;

    int message_len = message.size();
    const char* data = message.data();

    if (!connChannel_.isWriting()) {
        auto n = ::write(connfd_, data, message_len);
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
    if (!connChannel_.isWriting()) {
        connChannel_.enableWriting();
    } 
}

}