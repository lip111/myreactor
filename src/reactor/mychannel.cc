#include "mychannel.h"
#include "sys/epoll.h"
#include "myeventloop.h"

namespace myreactor {

// 构造函数
Channel::Channel(EventLoop* loop, int fd): loop_(loop), fd_(fd), 
    events_(0), revents_(0), eventHandling_(false) {}

Channel::~Channel() { disableAll(); }

void Channel::update() { 
    loop_->updateChannel(this); // channel依赖了eventloop,可优化！！！
}

void Channel::remove() {
    loop_->removeChannel(this);
}

bool Channel::isWriting() const { return events_ & EPOLLOUT; }

void Channel::setReadCallback(std::function<void()> cb) {
    readCallback_ = std::move(cb);
}

void Channel::setWriteCallback(std::function<void()> cb) {
    writeCallback_ = std::move(cb);
}

void Channel::setCloseCallback(std::function<void()> cb) {
    closeCallback_ = std::move(cb);
}

void Channel::setErrorCallback(std::function<void()> cb) {
    errorCallback_ = std::move(cb);
}

void Channel::handleEvent() {
    // 连接挂起并且无输入数据
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
        if (closeCallback_) closeCallback_();
    } 

    // 发生错误
    if (revents_ & EPOLLERR) {
        if (errorCallback_) errorCallback_();
    }

    // 有输入或者对端写关闭
    if (revents_ & (EPOLLIN | EPOLLRDHUP)) {
        if (readCallback_) readCallback_();
    }

    // 允许输出
    if (revents_ & (EPOLLOUT)) {
        if (writeCallback_) writeCallback_();
    }
}

void Channel::enableReading() {
    events_ |= EPOLLIN;
    update();
}

void Channel::disableReading() {
    events_ &= ~EPOLLIN;
    update();
}

void Channel::enableWriting() {
    events_ |= EPOLLOUT;
    update();
}

void Channel::disableWriting() {
    events_ &= ~EPOLLOUT;
    update();
}

void Channel::disableAll() {
    events_ = 0;
    update();
}

;
}