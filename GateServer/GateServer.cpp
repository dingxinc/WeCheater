// GateServer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//


#include "Server.h"

int main()
{
	try {
		unsigned short port = static_cast<unsigned short>(8080);
		net::io_context ioc{ 1 };  // 底层默认一个线程跑
		boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
		signals.async_wait([&ioc](const boost::system::error_code& error, int signal_errno) {
			if (error) {  // 出错直接返回
				return;
			}

			// 没有出错，接收到 ctrl + c 或者 kill 信号，直接关闭 ioc 事件循环
			ioc.stop();
		});

		std::make_shared<Server>(ioc, port)->Start();  // 启动服务器
		std::cout << "GateServer listen on port " << port << std::endl;
		ioc.run();   // 启动事件轮询
	}
	catch (std::exception const& e) {
		std::cout << "exception is " << e.what() << std::endl;
	}
}
