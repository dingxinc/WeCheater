#include "Server.h"
#include "HttpConnection.h"
#include "AsioIOServicePool.h"

Server::Server(boost::asio::io_context& ioc, unsigned short& port) : _ioc(ioc), _acceptor(ioc, tcp::endpoint(tcp::v4(), port)) {

}

void Server::Start()
{
	auto self = shared_from_this();
	/* �첽�������ӣ����������� _socket �н� */
	// �������ӻ���ʹ�� ioc �����գ����Ǵ������Ӹ�Ϊʹ�����ӳأ������ٸ��� _socket
	auto& io_context = AsioIOServicePool::GetInstance()->GetIOService();
	std::shared_ptr<HttpConnection> new_conn = std::make_shared<HttpConnection>(io_context);  // io_context �ڲ����һ�� socket
	_acceptor.async_accept(new_conn->GetSocket(), [self, new_conn](beast::error_code ec) {
		try {
			if (ec) {  // �д��󣬶���������ӣ����ҽ���������������
				self->Start();
				return;
			}

			// ���������ӣ������� HttpConnection ��ͳһ��������
			new_conn->Start();
			// ������������
			self->Start();
		}
		catch (std::exception& exp) {
			std::cout << exp.what() << std::endl;
		}
	});
}
