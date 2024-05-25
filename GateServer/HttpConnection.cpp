#include "HttpConnection.h"
#include "LogicSystem.h"

HttpConnection::HttpConnection(tcp::socket socket) : _socket(std::move(socket))
{
}

void HttpConnection::Start()
{
	auto self = shared_from_this();
	/* �첽����_request Ϊ�ͻ��˷��͵� http ����*/
	http::async_read(_socket, _buffer, _request, [self](beast::error_code ec, std::size_t bytes_transferred) {
		try {
			if (ec) {  // ��������
				std::cout << "http error is " << ec.what() << std::endl;
				return;
			}

			boost::ignore_unused(bytes_transferred);  // �Ѿ����͵��ֽ������Ե���http ����������Ҫ��ճ������
			self->HandleRequest();                    // ����ͻ��˷��͵� http ����
			self->CheckDeadline();                    // ������ʱ���
		}
		catch (std::exception& exp) {
			std::cout << "exception is " << exp.what() << std::endl;
		}
	});
}

void HttpConnection::CheckDeadline() // ��ʱ���� socket �ڵײ�û������socket �Ƕ�д�¼�����ʱ���Ƕ�ʱ�¼�
{
	auto self = shared_from_this();
	deadline_.async_wait([self](beast::error_code ec) {  // �����ӽ��� 60 s �󣬵ȵ��ˣ��ʹ����ص��������ر�����
		if (!ec) {  // û�г���
			self->_socket.close(ec);
		}
	});
}

void HttpConnection::WriteResponse()
{
	auto self = shared_from_this();
	_response.content_length(_response.body().size());   // ���ð���ĳ��ȣ��ײ���ݳ����а���ʵ��Ҳ�Ǵ���ճ��
	http::async_write(_socket, _response, [self](beast::error_code ec, std::size_t bytes_transferred) {  // ����� _socket ��д _response
		self->_socket.shutdown(tcp::socket::shutdown_send, ec);  // �رշ��Ͷˣ�ֻҪ�����˷��͵Ļص����������Ƿ�������ˣ����۶Է���û���յ������Ƕ�Ҫ�رգ��������ص�
		self->deadline_.cancel();   // ȡ����ʱ��
	});
}

void HttpConnection::HandleRequest()
{
	/* �Է������������Я���汾��������汾Ӧ���ȥ */
	_response.version(_request.version());
	/* ���ö����ӣ�http ��һ�ֶ����ӣ�tcp ��һ�ֳ����� */
	_response.keep_alive(false);

	// ���� get ����
	if (_request.method() == http::verb::get) {
		PreParseGetParam();   // ��������
		bool success = LogicSystem::GetInstance()->HandleGet(_get_url, shared_from_this());  // _request.target() �൱������� url
		if (!success) {
			_response.result(http::status::not_found);  // 404
			_response.set(http::field::content_type, "text/plain");  // �ı����͵�����
			beast::ostream(_response.body()) << "url not found\r\n"; // ��Ӧ�����д
			WriteResponse();                                         // Ӧ����ͻ���
			return;
		}

		_response.result(http::status::ok);
		_response.set(http::field::server, "GateServer");            // �������ĸ���������Ӧ��
		WriteResponse();
		return;
	}

	// ���� post ����
	if (_request.method() == http::verb::post) {
		bool success = LogicSystem::GetInstance()->HandlePost(_request.target(), shared_from_this());  // _request.target() �൱������� url
		if (!success) {
			_response.result(http::status::not_found);  // 404
			_response.set(http::field::content_type, "text/plain");  // �ı����͵�����
			beast::ostream(_response.body()) << "url not found\r\n"; // ��Ӧ�����д
			WriteResponse();                                         // Ӧ����ͻ���
			return;
		}

		_response.result(http::status::ok);
		_response.set(http::field::server, "GateServer");            // �������ĸ���������Ӧ��
		WriteResponse();
		return;
	}
}

void HttpConnection::PreParseGetParam()
{
	// ��ȡ URI  
	auto uri = _request.target();
	// ���Ҳ�ѯ�ַ����Ŀ�ʼλ�ã��� '?' ��λ�ã�  
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
			key = UrlDecode(pair.substr(0, eq_pos)); // ������ url_decode ����������URL����  
			value = UrlDecode(pair.substr(eq_pos + 1));
			_get_params[key] = value;
		}
		query_string.erase(0, pos + 1);
	}
	// �������һ�������ԣ����û�� & �ָ�����  
	if (!query_string.empty()) {
		size_t eq_pos = query_string.find('=');
		if (eq_pos != std::string::npos) {
			key = UrlDecode(query_string.substr(0, eq_pos));
			value = UrlDecode(query_string.substr(eq_pos + 1));
			_get_params[key] = value;
		}
	}
}
