#include "Server.h"
#include "HttpConnection.h"

Server::Server(boost::asio::io_context& ioc, unsigned short& port) : _ioc(ioc), _acceptor(ioc, tcp::endpoint(tcp::v4(), port)), _socket(ioc) {

}

void Server::Start()
{
	auto self = shared_from_this();
	/* �첽�������ӣ����������� _socket �н� */
	_acceptor.async_accept(_socket, [self](beast::error_code ec) {
		try {
			if (ec) {  // �д��󣬶���������ӣ����ҽ���������������
				self->Start();
				return;
			}

			// ���������ӣ������� HttpConnection ��ͳһ��������
			std::make_shared<HttpConnection>(std::move(self->_socket))->Start();
			// ������������
			self->Start();
		}
		catch (std::exception& exp) {
			std::cout << exp.what() << std::endl;
		}
	});
}
