#ifndef INET_BASE_H__
#define INET_BASE_H__

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <functional>
#include <vector>

/* 网络地址及端口封装 */
class InetAddress {
public:
    struct sockaddr_in addr;
    socklen_t addr_len;

public:
    InetAddress();
    InetAddress(const char* ip, uint16_t port);
    ~InetAddress();

    void setInetAddr(sockaddr_in _addr, socklen_t _addr_len);
    sockaddr_in getAddr();
    socklen_t getAddr_len();
};

/* 套接字类 */
class Socket {
private:
    int fd;

public:
    Socket();
    Socket(int);
    ~Socket();

    void bind(InetAddress*);
    void listen();
    int accept(InetAddress*);

    int getFd();
};

/* channel 类负责托管唯一的 fd */
class EventLoop;
class Channel {
private:
    EventLoop *loop; // eventloop 中有 epoll 对象， 相当于向上封装了一层
    int fd;          // channel 负责托管的 fd, fd 和 channel 是一对一的关系
    uint32_t events; // 负责监听的事件
    uint32_t revents;// 已经发生并返回的事件
    bool inEpoll;    // 标志位
    std::function<void()> callback; // 回调函数

public:
    Channel(EventLoop *_loop, int _fd);
    ~Channel();

    int getFd();

    uint32_t getEvents();
    uint32_t getRevents();

    void setRevents(uint32_t);

    bool getInEpoll();
    void setInEpoll();

    void handleEvent();
    void enableReading();

    void setCallback(std::function<void()>);
};

/* Epoll 类封装 */
class Epoll {
private:
    int epfd;
    struct epoll_event* events;

public:
    Epoll();
    ~Epoll();

    void addFd(int _fd, uint32_t _op);
    void updateChannel(Channel*);  // 更新 channel
    std::vector<Channel*> poll(int timeout = -1);
};

/* 事件驱动 */
class EventLoop {
private:
    Epoll *ep;
    bool quit;       // 事件循环是否停止的标志位

public:
    EventLoop();
    ~EventLoop();

    void loop();
    void updateChannel(Channel*);
};

/* 服务器 */
class Server {
private:
    EventLoop *loop;

public:
    Server(EventLoop *loop);
    ~Server();

    void handleReadEvent(int);
    void newConnection(Socket *servsock);
};


#endif // !INET_BASE_H__