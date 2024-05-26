#include "AsioIOServicePool.h"

AsioIOServicePool::~AsioIOServicePool()
{
    Stop();
    std::cout << "AsioIOServicePool destruct" << std::endl;
}

boost::asio::io_context& AsioIOServicePool::GetIOService()
{
    auto& service = _ioServices[_nextIOService++];
    if (_nextIOService == _ioServices.size()) {   // �������� size ��ȵ�ʱ������Ҫ��������� 0
        _nextIOService = 0;
    }
    return service;
}

void AsioIOServicePool::Stop()
{
    //��Ϊ����ִ��work.reset��������iocontext��run��״̬���˳�
    //��iocontext�Ѿ����˶���д�ļ����¼��󣬻���Ҫ�ֶ�stop�÷���
    for (auto& work : _works) {
        //�ѷ�����ֹͣ
        work->get_io_context().stop();  // ��ֹͣ work �� ����
        work.reset();    // ��ɿ�ָ�룬����� work
    }

    for (auto& t : _threads) {
        t.join();
    }
}

AsioIOServicePool::AsioIOServicePool(std::size_t size) : _ioServices(2), _works(2), _nextIOService(0) {
    for (std::size_t i = 0; i < size; ++i) {
        _works[i] = std::unique_ptr<Work>(new Work(_ioServices[i]));  // ��һ�������Ĵ��� work, ��������һ����Ӧ�� work
    }

    //�������ioservice����������̣߳�ÿ���߳��ڲ�����ioservice
    for (std::size_t i = 0; i < _ioServices.size(); ++i) {
        _threads.emplace_back([this, i]() {
            /* ioservice �� work �󶨣����ײ�û���¼���ʱ��run ����ֱ���˳����������ѯ��������󶨣��ͻ������˳���ѯ */
            _ioServices[i].run();   // ÿ���̶߳�Ӧһ�� ioservice�����÷���������
        });
    }
}