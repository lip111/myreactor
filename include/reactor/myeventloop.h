#pragma once
#include <memory>
#include <mychannel.h>
#include <mutex>
#include <functional>
#include <vector>
#include <thread>

namespace myreactor {

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
    int updateChannel(Channel* channel);
    int removeChannel(Channel* channel);

    // 跨线程任务投递
    void queueInLoop(std::function<void()> cb);

    // 执行任务，如果是当前线程的任务就执行，否则投递到目标线程的任务队列
    void runInLoop(std::function<void()> cb);

    // 判断当前执行代码的线程是不是eventloop所属的线程
    bool isInLoopThread() const;

    std::thread::id threadId() const;

private:
    void wakeup();
    void handleRead();
    void doPendingFunctors();


    bool quit_; // 控制停止事件循环
    std::unique_ptr<Poller> poller_;    // 使用独占指针可以不用知道Poller的完整声明 Poller poller_;不行
    
    int wakeupFd_;   // 专门用于跨线程唤醒
    Channel wakeupChannel_; // 注册eventfd到epoll实例

    std::vector<std::function<void()>> pendingFunctors_;    // 任务队列,处理跨线程投递过来的任务
    std::mutex mutex_;  // 避免多线程投递任务发生数据竞争

    std::thread::id threadId_;  // 当前线程id
};

}