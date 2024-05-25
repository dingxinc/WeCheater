#include "Server.h"
#include "HttpConnection.h"

Server::Server(boost::asio::io_context& ioc, unsigned short& port) : _ioc(ioc), _acceptor(ioc, tcp::endpoint(tcp::v4(), port)), _socket(ioc) {

}

void Server::Start()
{
	auto self = shared_from_this();
	/* 异步接收连接，将新连接用 _socket 承接 */
	_acceptor.async_accept(_socket, [self](beast::error_code ec) {
		try {
			if (ec) {  // 有错误，丢弃这个连接，并且接续监听其他连接
				self->Start();
				return;
			}

			// 创建新连接，并且让 HttpConnection 类统一管理连接
			std::make_shared<HttpConnection>(std::move(self->_socket))->Start();
			// 继续监听连接
			self->Start();
		}
		catch (std::exception& exp) {
			std::cout << exp.what() << std::endl;
		}
	});
}
