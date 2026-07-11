#pragma once
#include <vector>

namespace myreactor {
class Channel;

class Poller {
public:
    virtual ~Poller() = default;
    virtual int poll(int timeoutMs,  std::vector<Channel*>& activeChannels) = 0;
    virtual int updateChannel(Channel* channel) = 0;
    virtual int removeChannel(Channel* channel) = 0;
};

}

