#pragma once
#include <memory>

namespace myreactor {

class Channel;
class Poller;

class EventLoop {
public:
    // 构造函数
    EventLoop();
    ~EventLoop();

    // 查询
    void loop();

    // 设置
    void quit();
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);

private:
    bool quit_; // 控制停止事件循环
    std::unique_ptr<Poller> poller_;    // 使用独占指针可以不用知道Poller的完整声明 Poller poller_;不行
};

}