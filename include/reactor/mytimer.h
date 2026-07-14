#pragma once
#include <functional>
#include <chrono>

namespace myreactor {
using TimeCallback = std::function<void()>;

class Timer {
public:
    Timer(TimeCallback callback, double when, double interval, int id)
        :callback_(callback), 
        expiration_(std::chrono::steady_clock::now() + 
            std::chrono::milliseconds(static_cast<int>(when * 1000))),
        interval_(interval),
        repeat_(interval_ > 0.0),   
        id_(id)
        {}
    
    void run() const {
        callback_();
    }

    void restart() {
        if (repeat_) {
            expiration_ = std::chrono::steady_clock::now() + 
            std::chrono::microseconds(static_cast<int>(interval_ * 1000));
        }
    }

    int id() const {
        return id_;
    }

    std::chrono::steady_clock::time_point expiration() const {
        return expiration_;
    }

    bool repeat() const {
        return repeat_;
    }

    double interval() const {
        return interval_;
    }

private:
    TimeCallback callback_;
    std::chrono::steady_clock::time_point expiration_;
    double interval_;
    bool repeat_;
    int id_;
};

}