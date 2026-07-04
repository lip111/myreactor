#pragma once
#include <vector>
#include <sys/epoll.h>
#include <unordered_map>

namespace myreactor {

class Channel;

class Poller {
public:
    // 构造函数
    Poller();
    ~Poller();

    // 查询

    int poll(int timeoutMs, std::vector<Channel*> activeChannels); 

    // 设置

    // 添加或者修改一个fd在epoll中的注册状态
    void updateChannel(Channel* channel);

    // 移除一个注册的fd
    void removeChannel(Channel* channel);


private:
    static const int kInitEventListSize;
    int epollfd_;
    std::vector<epoll_event> events_;
    std::unordered_map<int, Channel*> channels_;    // 记录fd是否被注册
};


}