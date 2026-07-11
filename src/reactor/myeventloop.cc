#include "myeventloop.h"
#include "mypoller.h"
#include "mychannel.h"
#include <sys/eventfd.h>
#include <unistd.h>

namespace myreactor {

// 构造函数
EventLoop::EventLoop(): quit_(false), 
        poller_(std::make_unique<EpollPoller>()),
        wakeupFd_(::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)),
        wakeupChannel_(this, wakeupFd_),
        threadId_(std::this_thread::get_id())
{
    wakeupChannel_.setReadCallback([this](){ handleRead(); });
    wakeupChannel_.enableReading();
}

EventLoop::~EventLoop() {
    wakeupChannel_.remove();    // 注意从epoll实例移除注册的fd！！！
    ::close(wakeupFd_);
}

std::thread::id EventLoop::threadId() const {
    return threadId_;
}

// 查询
// 事件循环主函数
void EventLoop::loop() {
    while(!quit_) {
        std::vector<Channel*> activeChannels;
        int numEvents = poller_->poll(10000, activeChannels);

        // 先处理任务队列中的任务
        doPendingFunctors();

        for(int i = 0; i < numEvents; ++i) {
            activeChannels[i]->handleEvent();
        }
    }
}

int EventLoop::updateChannel(Channel* channel) {
    return poller_->updateChannel(channel);
}

int EventLoop::removeChannel(Channel* channel) {
    return poller_->removeChannel(channel);
}

void EventLoop::quit() { 
    quit_ = true; 
    wakeup();   // 可能跨线程终止线程，需要唤醒避免线程卡在epoll_wait！！！
}


void EventLoop::doPendingFunctors() {
    // 如果直接在锁内遍历执行任务，而某个任务内部又调用了 queueInLoop(再次尝试加锁)，就会导致死锁
    // 通过swap把vector内部数组移出去，既减少了加锁时间，又避免了死锁
    std::vector<std::function<void()>> functors;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        std::swap(functors, pendingFunctors_);
    }

    for(auto functor: functors) {
        functor();
    }

}

void EventLoop::handleRead() {
    uint64_t one = 0;
    ::read(wakeupFd_, &one, sizeof(one));

}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ::write(wakeupFd_, &one, sizeof(one));
}

void EventLoop::queueInLoop(std::function<void()> cb) {

    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingFunctors_.push_back(std::move(cb));
    }
    wakeup();
}

void EventLoop::runInLoop(std::function<void()> cb) {
    if (isInLoopThread())
        cb();
    else 
        queueInLoop(std::move(cb));
}

bool EventLoop::isInLoopThread() const {
    return threadId_ == std::this_thread::get_id();
}



}

