#pragma once
#include "global.h"

class HttpConnection : public std::enable_shared_from_this<HttpConnection>
{
	friend class LogicSystem;   // 因为 LogicSystem 要访问 HttpConnection 的私有成员变量
public:
	HttpConnection(tcp::socket socket);
	void Start();

private:
	void CheckDeadline();    // 超时检测
	void WriteResponse();    // 回复请求
	void HandleRequest();    // 处理请求
	void PreParseGetParam(); // 参数解析

private:
	tcp::socket _socket;
	beast::flat_buffer _buffer{ 8192 };   // 接收数据的数组
	http::request<http::dynamic_body> _request;  // 接收 http 请求
	http::response<http::dynamic_body> _response; // 回复请求
	net::steady_timer deadline_{
		_socket.get_executor(),  // 拿到 socket 的调度器
		std::chrono::seconds(60) // 60 s 的超时检测
	};

	std::string _get_url;
	std::unordered_map<std::string, std::string> _get_params;
};

