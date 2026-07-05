#include "InetAddress.h"
#include <arpa/inet.h>

namespace myreactor {

// 构造函数
InetAddress::InetAddress(): addr_{} {}

InetAddress::InetAddress(uint16_t port, bool loopbackOnly): addr_{} {   // 初始化列表中进行值初始化
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = htons(loopbackOnly ? INADDR_LOOPBACK: INADDR_ANY);
}

InetAddress::InetAddress(const std::string& ip, uint16_t port): addr_{} {
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr) <= 0) {
        addr_.sin_addr.s_addr = htons(INADDR_ANY);
    } 
}

InetAddress::InetAddress(const sockaddr_in& addr): addr_(addr) {}


// 查询
std::string InetAddress::toIp() const {
    char buf[64] = {0};
    inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    return buf;
}

std::string InetAddress::toIpPort() const {
    return toIp() + ":" + std::to_string(toPort());
}

// 设置
void InetAddress::setIp(const std::string& ip) {
    inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr);
}




;
}