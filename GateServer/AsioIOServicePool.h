#pragma once
#include "global.h"

class AsioIOServicePool : public Singleton<AsioIOServicePool>
{
	friend class Singleton<AsioIOServicePool>;
public:
	using IOService = boost::asio::io_context;
	using Work = boost::asio::io_context::work;
	using WorkPtr = std::unique_ptr<Work>;

	~AsioIOServicePool();
	AsioIOServicePool(const AsioIOServicePool&) = delete;
	AsioIOServicePool& operator=(const AsioIOServicePool&) = delete;
	boost::asio::io_context& GetIOService();
	void Stop();

private:
	AsioIOServicePool(std::size_t size = 2);

private:
	std::vector<IOService> _ioServices;   // 当我们传入一个 size, vector 会调用构造函数，构造好 size 个元素
	std::vector<WorkPtr> _works;
	std::vector<std::thread> _threads;
	std::size_t _nextIOService;
};

