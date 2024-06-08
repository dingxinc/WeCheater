#include "Session.h"
#include "Server.h"
#include <iostream>
#include <sstream>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include "LogicSystem.h"

Session::Session(boost::asio::io_context& io_context, Server* server):
	_socket(io_context), _server(server), _b_close(false),_b_head_parse(false){
	boost::uuids::uuid  a_uuid = boost::uuids::random_generator()();
	_uuid = boost::uuids::to_string(a_uuid);
	_recv_head_node = make_shared<MsgNode>(HEAD_TOTAL_LEN);
}

Session::~Session() {
	std::cout << "~Session destruct" << std::endl;
}

tcp::socket& Session::GetSocket() {
	return _socket;
}

std::string& Session::GetUuid() {
	return _uuid;
}

void Session::Start(){
	AsyncReadHead(HEAD_TOTAL_LEN);  // ���������յ�ʱ�����Ƚ���ͷ��
}

void Session::Send(std::string msg, short msgid) {
	std::lock_guard<std::mutex> lock(_send_lock);
	int send_que_size = _send_que.size();   // �첽����Ϊʲôʹ�ö��У������ܱ�֤�첽�������ݵ������ԣ�����һ�����ǽ���
	if (send_que_size > MAX_SENDQUE) {
		std::cout << "session: " << _uuid << " send que fulled, size is " << MAX_SENDQUE << std::endl;
		return;
	}

	_send_que.push(make_shared<SendNode>(msg.c_str(), msg.length(), msgid));
	if (send_que_size > 0) {
		return;
	}
	auto& msgnode = _send_que.front();
	boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
		std::bind(&Session::HandleWrite, this, std::placeholders::_1, SharedSelf()));// ������֮������ HandleWrite ����ص�����
}

void Session::Send(char* msg, short max_length, short msgid) {
	std::lock_guard<std::mutex> lock(_send_lock);
	int send_que_size = _send_que.size();
	if (send_que_size > MAX_SENDQUE) {
		std::cout << "session: " << _uuid << " send que fulled, size is " << MAX_SENDQUE << std::endl;
		return;
	}

	_send_que.push(make_shared<SendNode>(msg, max_length, msgid));
	if (send_que_size>0) {
		return;
	}
	auto& msgnode = _send_que.front();
	boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len), 
		std::bind(&Session::HandleWrite, this, std::placeholders::_1, SharedSelf()));
}

void Session::Close() {
	_socket.close();
	_b_close = true;
}

std::shared_ptr<Session>Session::SharedSelf() {
	return shared_from_this();
}

void Session::AsyncReadBody(int total_len)
{
	auto self = shared_from_this();   // �������ָ����������� Session ������ָ���ǹ������ü�����
	asyncReadFull(total_len, [self, this, total_len](const boost::system::error_code& ec, std::size_t bytes_transfered) {
		try {
			if (ec) {
				std::cout << "handle read failed, error is " << ec.what() << std::endl;
				Close();
				_server->ClearSession(_uuid);
				return;
			}

			if (bytes_transfered < total_len) {
				std::cout << "read length not match, read [" << bytes_transfered << "] , total ["
					<< total_len<<"]" << std::endl;
				Close();
				_server->ClearSession(_uuid);
				return;
			}

			memcpy(_recv_msg_node->_data , _data , bytes_transfered);
			_recv_msg_node->_cur_len += bytes_transfered;         // ��ǰ��ȡ���ֽ���һ��ƫ��
			_recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
			std::cout << "receive data is " << _recv_msg_node->_data << std::endl;
			//�˴�����ϢͶ�ݵ��߼�������
			LogicSystem::GetInstance()->PostMsgToQue(make_shared<LogicNode>(shared_from_this(), _recv_msg_node));
			//��������ͷ�������¼�
			AsyncReadHead(HEAD_TOTAL_LEN);
		}
		catch (std::exception& e) {
			std::cout << "Exception code is " << e.what() << endl;
		}
	});
}

void Session::AsyncReadHead(int total_len)
{
	auto self = shared_from_this();
	// ���������ʶ��ֻ�ж��� HEAD_TOTAL_LEN �ĳ���֮�󣬲Ż���ú���Ļص�����������û��ȫ���ǳ����˴����ʱ��Ż�ص��������
	// ʲôʱ���ȫ�ˣ�ʲôʱ��ص�����ĺ���
	asyncReadFull(HEAD_TOTAL_LEN, [self, this](const boost::system::error_code& ec, std::size_t bytes_transfered) {
		try {
			if (ec) {
				std::cout << "handle read failed, error is " << ec.what() << endl;
				Close();
				_server->ClearSession(_uuid);
				return;
			}

			if (bytes_transfered < HEAD_TOTAL_LEN) {
				std::cout << "read length not match, read [" << bytes_transfered << "] , total ["
					<< HEAD_TOTAL_LEN << "]" << endl;
				Close();
				_server->ClearSession(_uuid);
				return;
			}

			_recv_head_node->Clear();
			memcpy(_recv_head_node->_data, _data, bytes_transfered); // bytes_transfered ʵ�����Ƕ�ȡ�ĳ��ȣ�Ҳ����ͷ���ĳ���

			//��ȡͷ��MSGID����
			short msg_id = 0;
			memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LEN);
			//�����ֽ���ת��Ϊ�����ֽ���
			msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
			std::cout << "msg_id is " << msg_id << endl;
			//id�Ƿ�
			if (msg_id > MAX_LENGTH) {
				std::cout << "invalid msg_id is " << msg_id << endl;
				_server->ClearSession(_uuid);
				return;
			}
			short msg_len = 0;
			memcpy(&msg_len, _recv_head_node->_data + HEAD_ID_LEN, HEAD_DATA_LEN);
			//�����ֽ���ת��Ϊ�����ֽ���
			msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
			std::cout << "msg_len is " << msg_len << endl;

			//id�Ƿ�
			if (msg_len > MAX_LENGTH) {
				std::cout << "invalid data length is " << msg_len << endl;
				_server->ClearSession(_uuid);
				return;
			}

			_recv_msg_node = make_shared<RecvNode>(msg_len, msg_id);
			AsyncReadBody(msg_len);
		}
		catch (std::exception& e) {
			std::cout << "Exception code is " << e.what() << endl;
		}
	});
}

void Session::HandleWrite(const boost::system::error_code& error, std::shared_ptr<Session> shared_self) {
	//�����쳣����
	try {
		if (!error) {
			std::lock_guard<std::mutex> lock(_send_lock);
			//cout << "send data " << _send_que.front()->_data+HEAD_LENGTH << endl;
			_send_que.pop();
			if (!_send_que.empty()) {
				auto& msgnode = _send_que.front();
				boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
					std::bind(&Session::HandleWrite, this, std::placeholders::_1, shared_self));
			}
		}
		else {
			std::cout << "handle write failed, error is " << error.what() << endl;
			Close();
			_server->ClearSession(_uuid);
		}
	}
	catch (std::exception& e) {
		std::cerr << "Exception code : " << e.what() << endl;
	}
	
}

//��ȡ��������
void Session::asyncReadFull(std::size_t maxLength, std::function<void(const boost::system::error_code&, std::size_t)> handler )
{
	::memset(_data, 0, MAX_LENGTH);
	asyncReadLen(0, maxLength, handler);// �� _data �ĵ� 0 ��λ�ÿ�ʼ������ maxLength ������ȣ������ȫ�ˣ��ͻص� handler ����ص�����
}

//��ȡָ���ֽ���
void Session::asyncReadLen(std::size_t read_len, std::size_t total_len, // read_len ��ʾ�Ѿ������ֽ���������˵ֱ�Ӷ��� 5 ���ֽڣ���ô��һ�ξʹ��±�Ϊ 5 ��λ�ÿ�ʼ��
	std::function<void(const boost::system::error_code&, std::size_t)> handler)
{
	auto self = shared_from_this();
	// _data + read_len ��ʾ�Ѿ����ģ� total_len - read_len ��ʾδ����
	_socket.async_read_some(boost::asio::buffer(_data + read_len, total_len - read_len),// ����һ�� buffer, ��һ���������Ѿ��������ݣ��ڶ���������δ��������
		[read_len, total_len, handler, self](const boost::system::error_code& ec, std::size_t  bytesTransfered) {
			if (ec) {
				// ���ִ��󣬵��ûص�����
				handler(ec, read_len + bytesTransfered);// �ϴζ�ȡ�ĳ��� + ��ζ�ȡ�ĳ��� = ����Ѿ���ȡ�ĳ���
				return;
			}

			if (read_len + bytesTransfered >= total_len) {// �Ѿ���ȡ�ĳ��� + ��ζ�ȡ�ĳ���
				//���ȹ��˾͵��ûص�����
				handler(ec, read_len + bytesTransfered);
				return;
			}

			// û�д����ҳ��Ȳ����������ȡ
			self->asyncReadLen(read_len + bytesTransfered, total_len, handler);
	});
}

LogicNode::LogicNode(shared_ptr<Session>  session, 
	shared_ptr<RecvNode> recvnode):_session(session),_recvnode(recvnode) {
	
}
