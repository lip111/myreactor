#pragma once
#include <vector>
#include <sys/epoll.h>
#include <unordered_map>
#include <abstractpoller.h>

namespace myreactor {

class Channel;

class EpollPoller: public Poller {
public:
    // 构造函数
    EpollPoller();
    ~EpollPoller() override;

    // 查询

    int poll(int timeoutMs, std::vector<Channel*>& activeChannels) override;

    // 设置

    // 添加或者修改一个fd在epoll中的注册状态
    int updateChannel(Channel* channel) override;

    // 移除一个注册的fd
    int removeChannel(Channel* channel) override;


private:
    inline static const int kInitEventListSize = 16;
    int epollfd_;
    std::vector<epoll_event> events_;
    std::unordered_map<int, Channel*> channels_;    // 记录fd是否被注册
};


}