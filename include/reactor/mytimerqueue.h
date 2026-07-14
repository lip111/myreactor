#pragma once
#include <vector>
#include <memory>
#include "mytimer.h"
#include "mychannel.h"

namespace myreactor {
class EventLoop;

class TimerQueue {
public:
    TimerQueue(EventLoop* loop);
    ~TimerQueue();

    int addTimer(TimeCallback cb, double when, double interval);
    void cancelTimer(int id);

private:
    void handleRead();
    void resetTimerfd();

    EventLoop* loop_;
    std::vector<std::unique_ptr<Timer>> timers_;
    int timerfd_;
    Channel channel_;
    int nextTimerId_ = 1;   // 类内初始化,每个定时器唯一ID
};

}