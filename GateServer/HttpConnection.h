#pragma once
#include "global.h"

class HttpConnection : public std::enable_shared_from_this<HttpConnection>
{
	friend class LogicSystem;   // ��Ϊ LogicSystem Ҫ���� HttpConnection ��˽�г�Ա����
public:
	HttpConnection(tcp::socket socket);
	void Start();

private:
	void CheckDeadline();    // ��ʱ���
	void WriteResponse();    // �ظ�����
	void HandleRequest();    // ��������
	void PreParseGetParam(); // ��������

private:
	tcp::socket _socket;
	beast::flat_buffer _buffer{ 8192 };   // �������ݵ�����
	http::request<http::dynamic_body> _request;  // ���� http ����
	http::response<http::dynamic_body> _response; // �ظ�����
	net::steady_timer deadline_{
		_socket.get_executor(),  // �õ� socket �ĵ�����
		std::chrono::seconds(60) // 60 s �ĳ�ʱ���
	};

	std::string _get_url;
	std::unordered_map<std::string, std::string> _get_params;
};

