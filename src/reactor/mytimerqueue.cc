#include "mytimerqueue.h"
#include <sys/timerfd.h>
#include <unistd.h>
#include <memory>
#include <mylogger.h>
#include <cerrno>
#include <algorithm>

namespace myreactor {

TimerQueue::TimerQueue(EventLoop *loop): loop_(loop),
    timerfd_(::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)),
    channel_(loop, timerfd_)
{
    channel_.setReadCallback([this](){ handleRead(); });
    channel_.enableReading();
    LOG_DEBUG << "TimeQueue constructed, timerfd = " << timerfd_;
}

TimerQueue::~TimerQueue() {
    channel_.remove();
    ::close(timerfd_);
}

int TimerQueue::addTimer(TimeCallback cb, double when, double interval) {

    int id = nextTimerId_++;
    auto timer = std::make_unique<Timer>(std::move(cb), when, interval, id);
    
    int earliest = true;    // 如果当前定时器到期时间更近，需更新timerfd
    if (!timers_.empty() && timer->expiration() > timers_.front()->expiration())
        earliest = false;

    // LOG_DEBUG << "addTimer id = " << id << ", earlist = " << earliest << ", heap_size = " << timers_.size()
    //         << ", when = " << when << ", interval = " << interval;

    timers_.push_back(std::move(timer));
    std::push_heap(timers_.begin(), timers_.end(), 
        [](const std::unique_ptr<Timer>& a, const std::unique_ptr<Timer>& b){
            return a->expiration() > b->expiration();
    });
    
    if (earliest)   // 更新超时时间
        resetTimerfd();

    return id;
}

void TimerQueue::cancelTimer(int id) {
    for(auto it = timers_.begin(); it != timers_.end(); ++it) {
        if ((*it)->id() == id) {
            bool deleteHeapTop = (*it == timers_.front());
            timers_.erase(it);
            // 堆被破坏，重新建堆
            std::make_heap(timers_.begin(), timers_.end(),
                [](const std::unique_ptr<Timer>& a, const std::unique_ptr<Timer>& b){
                    return a->expiration() > b->expiration();
            });
            if (deleteHeapTop)  // 原来的堆顶没了,timerfd需更新
                resetTimerfd();
            return;
        }
    }
}

void TimerQueue::resetTimerfd() {

    if (timers_.empty())
        return;

    auto now = std::chrono::steady_clock::now();
    auto expiration = timers_.front()->expiration();
    auto diff = expiration - now;

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(expiration-now);
    if (duration.count() < 0) {
        duration = std::chrono::milliseconds(0);
    }

    // LOG_DEBUG << "resetTimerfd: duration = " << duration.count() << " ms";

    timespec ts;
    ts.tv_sec = duration.count() / 1000;
    ts.tv_nsec = (duration.count() % 1000) * 1000000;
    itimerspec value = {0};
    value.it_value = ts;

    int ret = ::timerfd_settime(timerfd_, 0, &value, nullptr);
    if (ret < 0) {
        LOG_ERROR << "timerfd_settime failed, errno = " << errno;
    }
}

void TimerQueue::handleRead() {
    uint64_t cnt = 0;
    ssize_t n = ::read(timerfd_, &cnt, sizeof(cnt));
    if (n != sizeof(cnt)) {
        LOG_ERROR << "TimerQueue::handleRead failed, fd = " << timerfd_ << ", errno = " << errno;
        return;
    }

    auto now = std::chrono::steady_clock::now();
    while(!timers_.empty() && timers_.front()->expiration() <= now) {
        std::pop_heap(timers_.begin(), timers_.end(), 
            [](const std::unique_ptr<Timer>& a, const std::unique_ptr<Timer>& b){
                return a->expiration() > b->expiration();
        });

        auto timer = std::move(timers_.back());
        timers_.pop_back();
        
        timer->run();   // 定时器回调

        if (timer->repeat()) {    // 如果是重复定时器，还需重新入堆
            timer->restart();
            timers_.push_back(std::move(timer));
            std::push_heap(timers_.begin(), timers_.end(), 
                [](const std::unique_ptr<Timer>& a, const std::unique_ptr<Timer>& b){
                    return a->expiration() > b->expiration();
                }
            );
        }
    }

    if(!timers_.empty())
        resetTimerfd();
}



}