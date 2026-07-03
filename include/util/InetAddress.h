#pragma once
#include <netinet/in.h>
#include <sys/socket.h>
#include <string>

namespace myreactor {

class InetAddress {

public:
    // 构造函数
    explicit InetAddress(uint16_t port, bool loopbackOnly=false);
    InetAddress(const std::string& ip, uint16_t port); 
    explicit InetAddress(const sockaddr_in& addr);

    // 查询
    std::string toIp() const;
    uint16_t toPort() const { return ntohs(addr_.sin_port); }
    std::string toIpPort() const;

    const sockaddr* getSockAddr() const { return reinterpret_cast<const sockaddr*>(&addr_); }
    sa_family_t family() const { return addr_.sin_family; }
    socklen_t socklen() const { return sizeof(addr_); }

    // 设置
    void setIp(const std::string& ip);
    void setPort(uint16_t port) { addr_.sin_port = htons(port); }
    void setAddr(const sockaddr_in& addr) { addr_ = addr; }


private:
    sockaddr_in addr_;
} 


;

}