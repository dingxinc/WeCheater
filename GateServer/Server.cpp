#include "Server.h"
#include "HttpConnection.h"
#include "AsioIOServicePool.h"

Server::Server(boost::asio::io_context& ioc, unsigned short& port) : _ioc(ioc), _acceptor(ioc, tcp::endpoint(tcp::v4(), port)) {

}

void Server::Start()
{
	auto self = shared_from_this();
	/* 异步接收连接，将新连接用 _socket 承接 */
	// 监听连接还是使用 ioc 来接收，但是处理连接该为使用连接池，而不再复用 _socket
	auto& io_context = AsioIOServicePool::GetInstance()->GetIOService();
	std::shared_ptr<HttpConnection> new_conn = std::make_shared<HttpConnection>(io_context);  // io_context 内部会绑定一个 socket
	_acceptor.async_accept(new_conn->GetSocket(), [self, new_conn](beast::error_code ec) {
		try {
			if (ec) {  // 有错误，丢弃这个连接，并且接续监听其他连接
				self->Start();
				return;
			}

			// 创建新连接，并且让 HttpConnection 类统一管理连接
			new_conn->Start();
			// 继续监听连接
			self->Start();
		}
		catch (std::exception& exp) {
			std::cout << exp.what() << std::endl;
		}
	});
}
