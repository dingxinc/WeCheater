#include "HttpConnection.h"
#include "LogicSystem.h"

HttpConnection::HttpConnection(tcp::socket socket) : _socket(std::move(socket))
{
}

void HttpConnection::Start()
{
	auto self = shared_from_this();
	/* 异步读，_request 为客户端发送的 http 请求*/
	http::async_read(_socket, _buffer, _request, [self](beast::error_code ec, std::size_t bytes_transferred) {
		try {
			if (ec) {  // 发生错误
				std::cout << "http error is " << ec.what() << std::endl;
				return;
			}

			boost::ignore_unused(bytes_transferred);  // 已经发送的字节数忽略掉，http 服务器不需要做粘包处理
			self->HandleRequest();                    // 处理客户端发送的 http 请求
			self->CheckDeadline();                    // 启动超时检测
		}
		catch (std::exception& exp) {
			std::cout << "exception is " << exp.what() << std::endl;
		}
	});
}

void HttpConnection::CheckDeadline() // 定时器和 socket 在底层没有区别，socket 是读写事件，定时器是定时事件
{
	auto self = shared_from_this();
	deadline_.async_wait([self](beast::error_code ec) {  // 当连接建立 60 s 后，等到了，就触发回调函数，关闭连接
		if (!ec) {  // 没有出错
			self->_socket.close(ec);
		}
	});
}

void HttpConnection::WriteResponse()
{
	auto self = shared_from_this();
	_response.content_length(_response.body().size());   // 设置包体的长度，底层根据长度切包，实际也是处理粘包
	http::async_write(_socket, _response, [self](beast::error_code ec, std::size_t bytes_transferred) {  // 往这个 _socket 里写 _response
		self->_socket.shutdown(tcp::socket::shutdown_send, ec);  // 关闭发送端，只要触发了发送的回调函数，就是发送完成了，无论对方接没接收到，我们都要关闭，短链接特点
		self->deadline_.cancel();   // 取消定时器
	});
}

void HttpConnection::HandleRequest()
{
	/* 对方请求过来，会携带版本，将这个版本应答回去 */
	_response.version(_request.version());
	/* 设置短链接，http 是一种短链接，tcp 是一种长连接 */
	_response.keep_alive(false);

	// 处理 get 请求
	if (_request.method() == http::verb::get) {
		PreParseGetParam();   // 解析参数
		bool success = LogicSystem::GetInstance()->HandleGet(_get_url, shared_from_this());  // _request.target() 相当于请求的 url
		if (!success) {
			_response.result(http::status::not_found);  // 404
			_response.set(http::field::content_type, "text/plain");  // 文本类型的请求
			beast::ostream(_response.body()) << "url not found\r\n"; // 往应答包里写
			WriteResponse();                                         // 应答给客户端
			return;
		}

		_response.result(http::status::ok);
		_response.set(http::field::server, "GateServer");            // 设置是哪个服务器的应答
		WriteResponse();
		return;
	}

	// 处理 post 请求
	if (_request.method() == http::verb::post) {
		bool success = LogicSystem::GetInstance()->HandlePost(_request.target(), shared_from_this());  // _request.target() 相当于请求的 url
		if (!success) {
			_response.result(http::status::not_found);  // 404
			_response.set(http::field::content_type, "text/plain");  // 文本类型的请求
			beast::ostream(_response.body()) << "url not found\r\n"; // 往应答包里写
			WriteResponse();                                         // 应答给客户端
			return;
		}

		_response.result(http::status::ok);
		_response.set(http::field::server, "GateServer");            // 设置是哪个服务器的应答
		WriteResponse();
		return;
	}
}

void HttpConnection::PreParseGetParam()
{
	// 提取 URI  
	auto uri = _request.target();
	// 查找查询字符串的开始位置（即 '?' 的位置）  
	auto query_pos = uri.find('?');
	if (query_pos == std::string::npos) {
		_get_url = uri;
		return;
	}

	_get_url = uri.substr(0, query_pos);
	std::string query_string = uri.substr(query_pos + 1);
	std::string key;
	std::string value;
	size_t pos = 0;
	while ((pos = query_string.find('&')) != std::string::npos) {
		auto pair = query_string.substr(0, pos);
		size_t eq_pos = pair.find('=');
		if (eq_pos != std::string::npos) {
			key = UrlDecode(pair.substr(0, eq_pos)); // 假设有 url_decode 函数来处理URL解码  
			value = UrlDecode(pair.substr(eq_pos + 1));
			_get_params[key] = value;
		}
		query_string.erase(0, pos + 1);
	}
	// 处理最后一个参数对（如果没有 & 分隔符）  
	if (!query_string.empty()) {
		size_t eq_pos = query_string.find('=');
		if (eq_pos != std::string::npos) {
			key = UrlDecode(query_string.substr(0, eq_pos));
			value = UrlDecode(query_string.substr(eq_pos + 1));
			_get_params[key] = value;
		}
	}
}
