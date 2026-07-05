#include "myeventloop.h"
#include "mypoller.h"
#include "mychannel.h"

namespace myreactor {

// 构造函数
EventLoop::EventLoop(): quit_(false), poller_(std::make_unique<Poller>()) {}
EventLoop::~EventLoop() {}

// 查询
// 事件循环主函数
void EventLoop::poll() {
    while(!quit_) {
        std::vector<Channel*> activeChannels;
        int numEvents = poller_->poll(10000, activeChannels);
        for(int i = 0; i < numEvents; ++i) {
            activeChannels[i]->handleEvent();
        }
    }
}

void EventLoop::updateChannel(Channel* channel) {
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel) {
    poller_->removeChannel(channel);
}

void EventLoop::quit() { quit_ = true; }
}