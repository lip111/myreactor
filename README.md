# 项目二：基于Reactor的多线程网络库

## 1. 项目背景

**一句话定位**：基于reactor模型的多线程C++网络库，支持高并发TCP连接管理与事件驱动

**技术栈**：C++17、epoll、多线程、网络socket、eventfd、timerfd

## 2. 整体架构和目录结构

### 2.1 整体架构
采用主从reactor模型：

- **主EventLoop**：负责监听新连接，通过Acceptor接收后，将新连接分配给子线程。
- **子EventLoop**：每个子线程运行一个EventLoop，管理一组TcpConnection，负责读写事件处理。
- **核心组件**：
  - EventLoop（事件循环）
  - Channel（fd+回调）
  - Poller（epoll封装）
  - Acceptor（监听新连接）
  - TcpConnection（连接管理）
  - Buffer（读写缓冲区）
  - TcpServer（顶层封装）
  - EventLoopThreadPool（线程池，管理子线程）
  - TimerQueue（定时器，基于timerfd）
  
### 2.2 目录结构

头文件位于 `include/`，源文件位于 `src/`，两者目录层次一致。各模块文件对应关系如下：

- **reactor/**（反应堆核心）
  - `myeventloop.h/.cc` — 事件循环引擎
  - `mychannel.h/.cc` — fd + 回调封装
  - `mypoller.h/.cc` — epoll 封装
  - `mytimerqueue.h/.cc` — 定时器管理

- **tcp/**（TCP 网络通信）
  - `myacceptor.h/.cc` — 监听新连接
  - `mybuffer.h/.cc` — 读写缓冲区
  - `mytcpconnection.h/.cc` — 连接管理
  - `mytcpserver.h/.cc` — 服务器顶层封装

- **thread/**（并发处理）
  - `myeventloopthread.h/.cc` — IO线程封装
  - `myeventloopthreadpool.h/.cc` — 线程池

- **util/**（基础工具）
  - `Socket.h/.cc` — 网络Socket系统调用封装
  - `Inetaddress.h/.cc` — IP/端口地址封装
  - `mylogger.h/.cc` — 日志输出

- **test/**（测试示例）
  - `echo_server.cc` — 回显服务器测试

- **根目录辅助脚本**
  - `build.sh` — 编译构建可执行程序
  - `check.sh` — 使用 valgrind 检查内存泄漏

其中 `build.sh` 负责调用 g++/cmake 编译项目，`check.sh` 通过 valgrind 运行测试程序，检测内存泄漏和非法访问。


## 3. 关键类说明

### 3.1 EventLoop
- **职责**：一个线程一个EventLoop，负责开启事件循环、处理跨线程投递的任务、执行channel回调。
- **关键成员**：Poller（epoll封装）、eventfd（用于跨线程唤醒）、TimerQueue（定时器）、pendingFunctors队列（跨线程任务）。
- **核心接口**：
  - `loop()`：运行事件循环，阻塞在epoll_wait。
  - `quit()`：退出事件循环。
  - `queueInLoop()`/`runInLoop()`：跨线程任务投递。
  - `updateChannel()`/`removeChannel()`：注册/移除fd事件。
  - `handleRead()`：处理eventfd读事件，主要是清空计数器，防止重复触发eventfd读。
  - `runAfter()`/`runEvery()`/`cancelTimer()`：启动一次性/周期性定时器，关闭定时器。

### 3.2 Channel
- **职责**：封装一个fd、关注的事件、事件对应的回调。当fd事件就绪，EventLoop会调用其`handleEvent()`。
- **关键成员**：`fd_`(文件描述符)、`events_`(关注的事件，比如EPOLLIN/EPOLLOUT)、`revents_`(Poller返回的实际发生的事件)、`loop_`(所属EventLoop的指针)、读/写/关闭/错误回调函数。
- **核心接口**：
  - `setReadCallback()`/`setWriteCallback()`/`setCloseCallback()`/`setErrorCallback()`：设置各类事件回调。
  - `enableReading()`/`disableReading()`/`enableWriting()`/`disableWriting()`：启动/禁用 读/写事件，通过更新(包括添加)Poller的fd实现。
  - `handleEvent()`：根据`revents_`调用对应的回调。
  - `remove()`：从Poller中移除自己，即删除对应的fd。

### 3.3 Poller
- **职责**：epoll的封装。负责将channel注册到epoll实例，以及从epoll中获取就绪的事件列表。
- **关键成员**：`epollfd_`(epoll实例的文件描述符)、`channels_`(fd到channel的映射表，记录已注册到epoll的fd)，`events_`(就绪的事件列表)。
- **核心接口**：
  - `poll()`：调用epoll_wait，返回就绪的channel列表。
  - `updateChannel()`：调用epoll_ctl添加或者修改fd的事件。
  - `removeChannel()`：调用epoll_ctl删除fd。
  
### 3.4 Acceptor
- **职责**：监听新连接。持有listenfd和对应的channel，当listenfd可读时调用`accept()`接收connfd，并通过回调通知TcpServer创建TcpConnection。
- **关键成员**：`socket_`(封装监听套接字，简化listenfd系统调用)、`channel_`(封装listenfd的channel)、`newConnectionCallback_`(接收连接后的回调，由TcpServer设置)、`loop_`(所属EventLoop的指针)
- **核心接口**：
  - `listen()`：开启监听端口。
  - `handleRead()`：调用`accept()`接受新连接，然后调用`newConnectionCallback_()`通知上层处理

### 3.5 TcpConnection
- **职责**：管理一个已建立的TCP连接，负责数据读写、缓冲区管理、连接关闭。
- **关键成员**：`socket_`(封装通信套接字connfd)、`channel_`(封装connfd的channel)、`inputBuffer_`(读缓冲区)、`outputBuffer_`(写缓冲区)、 `localaddr_`/`peeraddr_`(本端/对端地址)、`connected_`(连接状态)、`idleSeconds_`(空闲超时时间)、`idleTimerId_`(定时器id)、读/读完/关闭/错误回调函数、`loop_`(所属EventLoop的指针)
- **核心接口**：
  - `handleRead()`：将connfd接收缓冲区的数据读到inputBuffer_，调用用户回调。
  - `handleWrite()`：将outputBuffer_中的数据写入connfd发送缓冲区。
  - `handleClose()`：关闭连接，从epoll实例中移除connfd，并通知TcpServer删除自身。
  - `forceClose()`：强制关闭连接。
  - `send()`：发送数据，优先直接写入connfd发送缓冲区，如果写不完，放入outputBuffer_并注册写事件。
  - `connectEstablished()`：跨线程投递新连接任务中，由被投递线程处理任务时调用，注册新连接connfd到自己的epoll实例，并启动空闲超时定时器。
  
### 3.6 Buffer
  - **职责**：封装读写缓冲区，提供高效的读写接口，减少系统调用次数
  - **关键成员**：
    - `buffer_`：vector<char>,临时存储缓冲区数据
    - `readerIdx`：标记`buffer_`数组中可读位置，即该位置到`writerIdx`之间都是待处理的有效数据
    - `writerIdx`：标记可写位置，即该位置到`buffer_.size()`之间都是空位，可以存放数据
  - **核心接口**：
    - `readableBytes()`/`writableBytes()`：返回可读/可写字节数。
    - `append()`/`retrieve()`：写入数据/取出已读数据。
    - `peek()`：可读数据起始地址的指针
    - `readFd()`：readv系统调用将connfd接收缓冲区的数据一次性读到inputBuffer_中    

### 3.7 TcpServer
  - **职责**：服务器顶层封装，统筹管理Acceptor、线程池、连接池。负责启动监听、分配新连接到子线程、管理所有活跃连接的生命周期。
  - **关键成员**：`loop_`(主线程EvenLoop的指针)、`acceptor_`(Acceptor对象)、`threadPool_`(线程池对象)、`connections_`(活跃连接的map,存放"ip+port:tcpconnecton对象"的键值对)、用户回调
  - **核心接口**：
    - `start()`：启动acceptor监听和线程池
    - `newConnection()`：由Acceptor回调触发，从线程池中选一个子EventLoop，创建TcpConnection并进行跨线程投递。
    - `removeConnection()`：由TcpConnection关闭时回调触发，从`connections_`中移除连接。

### 3.8 EventLoopThreadPool
  - **职责**：管理一组子线程和对应的EventLoop，提供轮流分配策略，供TcpServer为新连接分配子线程。
  - **关键成员**：`mainLoop_`(主线程EventLoop指针)、`threads_`(子线程列表)、`loops_`(子线程专用eventloop指针列表)、`next_`(下一个分配的子线程eventloop索引，用于轮流分配)。
  - **核心接口**：
    - `start()`：用于创建指定数量的子线程和子线程的eventloop，并启动子线程Eventloop的事件循环。
    - `getNextLoop()`：分配下一个处理新连接的子线程EventLoop指针(轮流分配，第一个连接给线程1，第二个连接给线程2，以此类推，等到所有子线程都被分配一遍又重新回到线程1)
  
### 3.9 TimerQueue
  - **职责**：管理定时器，基于timerfd实现。提供创建一次性以及周期性定时器、取消定时器的功能。当timerfd可读，获取到期的定时器并执行对应的回调。
  - **关键成员**：`timerfd_`、`channel_`、`timers_`(定时器，最小堆结构存储)、`nextTimerId_`(下一个该分配的定时器id)、`loop_`(所属的eventloop指针)
  - **核心接口**：
    - `addTimer()`：添加定时器，更新最小堆，如果该定时器到期时间最短，更新timerfd的超时时间
    - `cancelTimer()`：删除定时器，更新最小堆，如果删除的定时器到期时间最短，更新timerfd的超时时间
    - `handleRead()`：timerfd可读时调用触发，取出所有到期的定时器，调用定时器的回调，更新最小堆和timerfd的超时时间