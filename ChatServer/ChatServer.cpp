﻿// ChatServer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "LogicSystem.h"
#include <csignal>
#include <thread>
#include <mutex>
#include "AsioIOServicePool.h"
#include "Server.h"
#include "ConfigMgr.h"

using namespace std;
bool bstop = false;

std::condition_variable cond_quit;
std::mutex mutex_quit;

int main()
{
	try {
		auto& cfg = ConfigMgr::GetInstance();
		auto pool = AsioIOServicePool::GetInstance();
		boost::asio::io_context  io_context;
		boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
		signals.async_wait([&io_context, pool](auto, auto) {
			io_context.stop();
			pool->Stop();
		});

		auto port_str = cfg["SelfServer"]["Port"];
		Server s(io_context, atoi(port_str.c_str()));
		io_context.run();// 如果 io_context 在 run 的时候没有绑定任何的事件，那么就会直接退出，在构造服务器的时候，绑定了一个异步监听的事件，所以这里不会退出
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << endl;
	}

	return 0;
}
