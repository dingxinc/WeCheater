#include "InetBase.h"
#include "util.h"
#include <string.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define MAX_EVENTS 1024
#define READ_BUFFER 1024

InetAddress::InetAddress() : addr_len(sizeof(addr)) {
    bzero(&addr, sizeof(addr));
}

InetAddress::InetAddress(const char* ip, uint16_t port) : addr_len(sizeof(addr)) {
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    addr_len = sizeof(addr);
}

InetAddress::~InetAddress() { }

void InetAddress::setInetAddr(sockaddr_in _addr, socklen_t _addr_len) {
    addr = _addr;
    addr_len = _addr_len;
}

sockaddr_in InetAddress::getAddr() {
    return addr;
}

socklen_t InetAddress::getAddr_len() {
    return addr_len;
}

/***********************************************************************************/

Socket::Socket() : fd(-1) {
    fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    errif(fd == -1, "sock create failed.");

    // 设置 listenfd 的属性
    int opt;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, static_cast<socklen_t>(sizeof(opt)));   // 必须的
    setsockopt(fd, SOL_SOCKET, TCP_NODELAY, &opt, static_cast<socklen_t>(sizeof(opt)));    // 必须的
    setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, static_cast<socklen_t>(sizeof(opt)));   // Reactor 中用处不大
    setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &opt, static_cast<socklen_t>(sizeof(opt)));   // 建议自己实现心跳机制
}

Socket::Socket(int _fd) : fd(_fd) {
    errif(fd == -1, "socket create error");
}

Socket::~Socket() {
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
}

void Socket::bind(InetAddress* _addr) {
    struct sockaddr_in addr = _addr->getAddr();
    socklen_t addr_len = _addr->getAddr_len();
    errif(::bind(fd, (sockaddr*)&addr, addr_len) == -1, "socket bind error");
    _addr->setInetAddr(addr, addr_len);
}

void Socket::listen() {
    errif(::listen(fd, SOMAXCONN) == -1, "listen error");
}

int Socket::accept(InetAddress* _addr) {
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    bzero(&addr, sizeof(addr));
    int clntsock = ::accept4(fd, (sockaddr*)&addr, &addr_len, SOCK_NONBLOCK);
    errif(clntsock == -1, "socket accept error");
    _addr->setInetAddr(addr, addr_len);
    return clntsock;
}

int Socket::getFd() {
    return fd;
}

/***********************************************************************************/

Channel::Channel(EventLoop* _loop, int _fd) : loop(_loop), fd(_fd), events(0), revents(0), inEpoll(false) {

}

Channel::~Channel() {

}

int Channel::getFd() {
    return fd;
}

uint32_t Channel::getEvents() {
    return events;
}

uint32_t Channel::getRevents() {
    return revents;
}

void Channel::setRevents(uint32_t _ev) {
    revents = _ev;
}

bool Channel::getInEpoll() {
    return inEpoll;
}

void Channel::setInEpoll() {
    inEpoll = true;
}

void Channel::handleEvent() {
    callback();
}

void Channel::enableReading() {
    events = EPOLLIN | EPOLLET;
    loop->updateChannel(this);
}

void Channel::setCallback(std::function<void()> _cb) {
    callback = _cb;
}

/*******************************************************************************************/

Epoll::Epoll() : epfd(-1), events(nullptr) {
    epfd = epoll_create1(0);
    errif(epfd == -1, "epoll_creates error.");
    events = new epoll_event[MAX_EVENTS];  // 注意这里 epoll_event 可以 new 出来
    // 这里第一个参数是一个指针，events 本身就是一个指针，所以就不是填 &events，而就是 events，不然 epoll_wait() 返回 -1
    // 初始化为 0 是一个好习惯，但是记住，对指针和数组的操作一定要小心再小心，不然会引发难以预料的错误
    bzero(events, sizeof(*events) * MAX_EVENTS);  // 这个一个数组指针
}

Epoll::~Epoll() {
    if (epfd != -1) {
        close(epfd);
        epfd = -1;
    }
    delete [] events;
}

void Epoll::addFd(int _fd, uint32_t _op) {
    struct epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.data.fd = _fd;
    ev.events = _op;
    errif(epoll_ctl(epfd, EPOLL_CTL_ADD, _fd, &ev) == -1, "epoll add event error");
}

void Epoll::updateChannel(Channel* channel) {
    int fd = channel->getFd();
    struct epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.data.ptr = channel;
    ev.events = channel->getEvents();    // 这里的 getEvents() 是在 Channel 类中的 enableReading() 中设置的
    if (!channel->getInEpoll()) {
        errif(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll_ctl error");
        channel->setInEpoll();        
    } else {  // 已经在树上，直接修改
        errif(epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1, "epoll_ctl error");
    }
}

std::vector<Channel*> Epoll::poll(int timeout) {
    std::vector<Channel*> channels;
    int nfds = epoll_wait(epfd, events, MAX_EVENTS, timeout);
    errif(nfds == -1, "epoll wait error");
    for (int i = 0; i < nfds; ++i) {
        Channel* ch = (Channel*)events[i].data.ptr;
        ch->setRevents(events[i].events);
        channels.push_back(ch);
    }
    return channels;
}

/*****************************************************************************************/

EventLoop::EventLoop() : ep(nullptr), quit(false) {
    ep = new Epoll();
}

EventLoop::~EventLoop() {
    delete ep;
}

void EventLoop::loop() {
    while (!quit) {
        std::vector<Channel*> chs;
        chs = ep->poll();
        for (auto it = chs.begin(); it != chs.end(); ++it) {
            (*it)->handleEvent();
        }
    }
}

void EventLoop::updateChannel(Channel* ch) {
    ep->updateChannel(ch);
}

/*********************************************************************************/

/* 完成监听套接字的创建和绑定监听 */
Server::Server(EventLoop *_loop) : loop(_loop) {
    Socket *servsock = new Socket();
    InetAddress *servaddr = new InetAddress("127.0.0.1", 8888);
    servsock->bind(servaddr);
    servsock->listen();

    Channel *servChannel = new Channel(loop, servsock->getFd());
    std::function<void()> cb = std::bind(&Server::newConnection, this, servsock);
    servChannel->setCallback(cb);
    servChannel->enableReading();
}

Server::~Server() {

}

/* 处理读事件 */
void Server::handleReadEvent(int sockfd) {
    char buf[READ_BUFFER];
    while (true) {       // 非阻塞模式，必须一次读完
        bzero(&buf, sizeof(buf));
        ssize_t read_bytes = read(sockfd, buf, sizeof(buf));
        if (read_bytes > 0) {
            printf("message from client fd %d : %s.\n", sockfd, buf);
            write(sockfd, buf, sizeof(buf));    
        } else if (read_bytes == -1 && errno == EINTR) {  // 信号中断，继续读
            printf("siglnal eintr, continue read...\n");
            continue;
        } else if (read_bytes == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {  // 非阻塞模式下，表示读完
            printf("read complete.\n");
            break;
        } else if (read_bytes == 0) {  // EOF, 客户端断开连接
            printf("EOF client sock fd %d, is disconnection.\n", sockfd);
            close(sockfd);  // 关闭文件描述符，从红黑树上移除
            break;
        }
    }
}

/* 处理新连接 */
void Server::newConnection(Socket *servsock) {
    InetAddress *clntaddr = new InetAddress();
    Socket *clntsock = new Socket(servsock->accept(clntaddr));
    printf("new client fd %d! IP: %s Port: %d\n", clntsock->getFd(), inet_ntoa(clntaddr->addr.sin_addr), ntohs(clntaddr->addr.sin_port));

    Channel *clntChannel = new Channel(loop, clntsock->getFd()); // loop 和 ep 没区别，就是一层简单的封装
    std::function<void()> cb = std::bind(&Server::handleReadEvent, this, clntsock->getFd());
    clntChannel->setCallback(cb);
    clntChannel->enableReading();      // 客户端的文件描述符也挂上去了
}