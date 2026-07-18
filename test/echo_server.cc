#include "mytcpserver.h"
#include "mytcpconnection.h"
#include "myeventloop.h"
#include "InetAddress.h"
#include <iostream>
#include <mylogger.h>
#include <csignal>

myreactor::EventLoop* g_eventloop = nullptr;
extern "C" void signalHandler(int) {
    if (g_eventloop)
        g_eventloop->quit();
}


void onConnection(const std::shared_ptr<myreactor::TcpConnection>& conn) {

    // if (conn->connected()) {
    //     std::cout << "[INFO] New connection established from "
    //         << conn->peeraddr().toIpPort() 
    //         << " thread id: " << conn->getLoop()->threadId() << std::endl;  
    // }
    // else {
    //     std::cout << "[INFO] Connection from "
    //         << conn->peeraddr().toIpPort() 
    //         << " thread id: " << conn->getLoop()->threadId()
    //         << " closed" << std::endl;
    // }
}

void onMessage(const std::shared_ptr<myreactor::TcpConnection>& conn, myreactor::Buffer* buffer) {
    
    std::size_t n = buffer->readableBytes();
    std::string msg(buffer->peek(), n);

    buffer->retrieve(n);
    // std::cout << "receive " << n << " bytes, msg: " << msg;
    conn->send(msg);
}

int main() {
    ::signal(SIGINT, signalHandler);
    ::signal(SIGTERM, signalHandler);

    myreactor::EventLoop loop;
    g_eventloop = &loop;

    myreactor::InetAddress listenAddr(8080);
    myreactor::TcpServer server(&loop, listenAddr);

    server.setMessageCallback(onMessage);
    server.setConnectionCallback(onConnection);

    // server.setThreadNum(3);
    server.start();

    loop.loop();
}