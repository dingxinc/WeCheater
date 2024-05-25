#pragma once
#include "global.h"

class Server : public std::enable_shared_from_this<Server>
{
public:
	Server(boost::asio::io_context& ioc, unsigned short& port);  // io_context 上下文，用来监听事件循环，windows 下底层是 iocp, linux 下底层是 epoll
	void Start();              // 服务器启动

private:
	tcp::acceptor _acceptor;   // 接收器，用来接收连接
	net::io_context& _ioc;
	tcp::socket _socket;       // 用来承接连接上来的 客户端
};

