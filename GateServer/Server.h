#pragma once
#include "global.h"

class Server : public std::enable_shared_from_this<Server>
{
public:
	Server(boost::asio::io_context& ioc, unsigned short& port);  // io_context �����ģ����������¼�ѭ����windows �µײ��� iocp, linux �µײ��� epoll
	void Start();              // ����������

private:
	tcp::acceptor _acceptor;   // ��������������������
	net::io_context& _ioc;
	tcp::socket _socket;       // �����н����������� �ͻ���
};

