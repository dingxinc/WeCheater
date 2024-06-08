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
	AsyncReadHead(HEAD_TOTAL_LEN);  // 服务器接收的时候首先接收头部
}

void Session::Send(std::string msg, short msgid) {
	std::lock_guard<std::mutex> lock(_send_lock);
	int send_que_size = _send_que.size();   // 异步发送为什么使用队列？队列能保证异步发送数据的有序性，另外一个就是解耦
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
		std::bind(&Session::HandleWrite, this, std::placeholders::_1, SharedSelf()));// 发完了之后会调用 HandleWrite 这个回调函数
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
	auto self = shared_from_this();   // 这个智能指针和其他管理 Session 的智能指针是共享引用计数的
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
			_recv_msg_node->_cur_len += bytes_transfered;         // 当前读取的字节做一次偏移
			_recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
			std::cout << "receive data is " << _recv_msg_node->_data << std::endl;
			//此处将消息投递到逻辑队列中
			LogicSystem::GetInstance()->PostMsgToQue(make_shared<LogicNode>(shared_from_this(), _recv_msg_node));
			//继续监听头部接受事件
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
	// 这个函数标识，只有读完 HEAD_TOTAL_LEN 的长度之后，才会调用后面的回调函数，或者没读全但是出现了错误的时候才会回调这个函数
	// 什么时候读全了，什么时候回调后面的函数
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
			memcpy(_recv_head_node->_data, _data, bytes_transfered); // bytes_transfered 实际上是读取的长度，也就是头部的长度

			//获取头部MSGID数据
			short msg_id = 0;
			memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LEN);
			//网络字节序转化为本地字节序
			msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
			std::cout << "msg_id is " << msg_id << endl;
			//id非法
			if (msg_id > MAX_LENGTH) {
				std::cout << "invalid msg_id is " << msg_id << endl;
				_server->ClearSession(_uuid);
				return;
			}
			short msg_len = 0;
			memcpy(&msg_len, _recv_head_node->_data + HEAD_ID_LEN, HEAD_DATA_LEN);
			//网络字节序转化为本地字节序
			msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
			std::cout << "msg_len is " << msg_len << endl;

			//id非法
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
	//增加异常处理
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

//读取完整长度
void Session::asyncReadFull(std::size_t maxLength, std::function<void(const boost::system::error_code&, std::size_t)> handler )
{
	::memset(_data, 0, MAX_LENGTH);
	asyncReadLen(0, maxLength, handler);// 从 _data 的第 0 个位置开始读，读 maxLength 这个长度，如果读全了，就回调 handler 这个回调函数
}

//读取指定字节数
void Session::asyncReadLen(std::size_t read_len, std::size_t total_len, // read_len 表示已经读了字节数，假如说直接读了 5 个字节，那么下一次就从下标为 5 的位置开始读
	std::function<void(const boost::system::error_code&, std::size_t)> handler)
{
	auto self = shared_from_this();
	// _data + read_len 表示已经读的， total_len - read_len 表示未读的
	_socket.async_read_some(boost::asio::buffer(_data + read_len, total_len - read_len),// 构造一个 buffer, 第一个参数是已经读的数据，第二个参数是未读的数据
		[read_len, total_len, handler, self](const boost::system::error_code& ec, std::size_t  bytesTransfered) {
			if (ec) {
				// 出现错误，调用回调函数
				handler(ec, read_len + bytesTransfered);// 上次读取的长度 + 这次读取的长度 = 这次已经读取的长度
				return;
			}

			if (read_len + bytesTransfered >= total_len) {// 已经读取的长度 + 这次读取的长度
				//长度够了就调用回调函数
				handler(ec, read_len + bytesTransfered);
				return;
			}

			// 没有错误，且长度不足则继续读取
			self->asyncReadLen(read_len + bytesTransfered, total_len, handler);
	});
}

LogicNode::LogicNode(shared_ptr<Session>  session, 
	shared_ptr<RecvNode> recvnode):_session(session),_recvnode(recvnode) {
	
}
