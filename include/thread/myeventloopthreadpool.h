#pragma once

#include <vector>
#include <memory>


namespace myreactor {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool {
public:
    EventLoopThreadPool(EventLoop* mainLoop);
    ~EventLoopThreadPool();

    void setThreadNum(int numThreads);
    int numThreads() const;

    // 创建并启动所有从线程
    void start();

    // 取出从线程的下一个EventLoop,用于分配新连接
    EventLoop* getNextLoop();

private:
    EventLoop* mainLoop_;
    std::vector<EventLoop*> slaveloops_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    int next_;  // 轮询计数器
    bool started_;  // 防止重复启动
    int numThreads_;

};


}