#pragma once
#include <functional>

namespace myreactor {

class EventLoop;

class Channel {
public:
    // 构造函数
    Channel(EventLoop* loop, int fd);
    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;
    ~Channel();

    // 查询
    int fd() { return fd_; }
    int events() const { return events_; }
    int revents() const { return revents_; }
    EventLoop* loop() const { return loop_; }
    bool isWriting() const;   // 确保EPOLLOUT事件发生确实是需要写数据

    // 设置
    void enableReading();
    void disableReading();
    void enableWriting();
    void disableWriting();
    void disableAll();

    void setReadCallback(std::function<void()> cb);
    void setWriteCallback(std::function<void()> cb);
    void setCloseCallback(std::function<void()> cb);
    void setErrorCallback(std::function<void()> cb);
    void handleEvent();

    void setRevents(int revents) { revents_ = revents; }
    
    void remove();


private:
    void update();

    EventLoop* loop_;
    int fd_;
    int events_;    // 感兴趣的事件
    int revents_;   // 实际发生的事件
    bool eventHandling_;    // 防止handleEvent被递归调用

    std::function<void()> readCallback_;
    std::function<void()> writeCallback_;
    std::function<void()> closeCallback_;
    std::function<void()> errorCallback_;
};

}