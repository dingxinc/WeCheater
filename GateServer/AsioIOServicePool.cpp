#include "AsioIOServicePool.h"

AsioIOServicePool::~AsioIOServicePool()
{
    Stop();
    std::cout << "AsioIOServicePool destruct" << std::endl;
}

boost::asio::io_context& AsioIOServicePool::GetIOService()
{
    auto& service = _ioServices[_nextIOService++];
    if (_nextIOService == _ioServices.size()) {   // 当索引和 size 相等的时候，马上要把索引变成 0
        _nextIOService = 0;
    }
    return service;
}

void AsioIOServicePool::Stop()
{
    //因为仅仅执行work.reset并不能让iocontext从run的状态中退出
    //当iocontext已经绑定了读或写的监听事件后，还需要手动stop该服务。
    for (auto& work : _works) {
        //把服务先停止
        work->get_io_context().stop();  // 先停止 work 再 回收
        work.reset();    // 变成空指针，会回收 work
    }

    for (auto& t : _threads) {
        t.join();
    }
}

AsioIOServicePool::AsioIOServicePool(std::size_t size) : _ioServices(2), _works(2), _nextIOService(0) {
    for (std::size_t i = 0; i < size; ++i) {
        _works[i] = std::unique_ptr<Work>(new Work(_ioServices[i]));  // 把一个上下文传给 work, 就能生成一个对应的 work
    }

    //遍历多个ioservice，创建多个线程，每个线程内部启动ioservice
    for (std::size_t i = 0; i < _ioServices.size(); ++i) {
        _threads.emplace_back([this, i]() {
            /* ioservice 和 work 绑定，当底层没有事件的时候，run 不会直接退出，会继续轮询，如果不绑定，就会立即退出轮询 */
            _ioServices[i].run();   // 每个线程对应一个 ioservice，并让服务跑起来
        });
    }
}