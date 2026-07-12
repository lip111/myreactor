#include "myeventloopthreadpool.h"
#include "myeventloopthread.h"
#include <thread>

namespace myreactor {

EventLoopThreadPool::EventLoopThreadPool(EventLoop* mainLoop)
    :mainLoop_(mainLoop), next_(0), started_(false),
    numThreads_(std::thread::hardware_concurrency()/2) {}


EventLoopThreadPool::~EventLoopThreadPool() = default;

void EventLoopThreadPool::setThreadNum(int numThreads) {
    numThreads_ = numThreads;
}

int EventLoopThreadPool::numThreads() const {
    return numThreads_;
}

void EventLoopThreadPool::start() {

    started_ = true;
    for(int i = 0; i < numThreads_; ++i) {
        auto slaveThread = std::make_unique<EventLoopThread>();
        auto slaveLoop = slaveThread->startLoop();
        slaveloops_.push_back(slaveLoop);
        threads_.push_back(std::move(slaveThread)); // 注意线程只能移动，不能拷贝！！！
    }
}

EventLoop* EventLoopThreadPool::getNextLoop() {
    
    // 若未创建子线程，新连接还是由主线程处理
    if (!started_)
        return mainLoop_;
    if (slaveloops_.empty())
        return mainLoop_;
    
    EventLoop* loop = slaveloops_[next_];
    next_ = (next_+1) % slaveloops_.size(); // 新连接均匀分布在所有子线程

    return loop;
}


}
