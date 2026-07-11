#include "mypoller.h"
#include <unistd.h>
#include "mychannel.h"
#include <cstdio>
#include <cerrno>


namespace myreactor {

// 构造函数

// EPOLL_CLOEXEC保证exec执行时关闭fd,防止被子进程继承
EpollPoller::EpollPoller(): epollfd_(epoll_create1(EPOLL_CLOEXEC)), 
    events_(EpollPoller::kInitEventListSize) {}

EpollPoller::~EpollPoller() {
    ::close(epollfd_);
}

// 查询

int EpollPoller::poll(int timeoutMs, std::vector<Channel*>& activeChannels) {    // 注意传递引用！！！

    int numEvents = epoll_wait(epollfd_, events_.data(), static_cast<int>(events_.size()), timeoutMs);
    if (numEvents > 0) {
        for(int i = 0; i < numEvents; ++i) {

            Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
            channel->setRevents(events_[i].events);
            activeChannels.push_back(channel);
        }

        // 就绪事件比较多，扩容
        if (numEvents == static_cast<int>(events_.size())) {
            events_.resize(events_.size() * 2);
        }
    }

    return numEvents;
}

int EpollPoller::updateChannel(Channel* channel) {

    int socketfd = channel->fd();
    epoll_event event;
    event.events = channel->events();
    event.data.ptr = channel;

    // 新的fd, 新注册事件
    if (auto it = channels_.find(socketfd); it == channels_.end()) {
        if(0 == ::epoll_ctl(epollfd_, EPOLL_CTL_ADD, socketfd, &event)) {
            channels_[socketfd] = channel;
            return 0;
        }
        else {
            return -1;
        }
    }
    // 旧fd, 更新事件
    else {
        if(0 != ::epoll_ctl(epollfd_, EPOLL_CTL_MOD, socketfd, &event)) {
            return -1;
        } 
        else {
            return 0;
        }
    }
}

int EpollPoller::removeChannel(Channel* channel) {
    int socketfd = channel->fd();
    if(0 == ::epoll_ctl(epollfd_, EPOLL_CTL_DEL, socketfd, nullptr)) {
        channels_.erase(socketfd);
        return 0;
    }
    else {
        return -1;
    }
}

}