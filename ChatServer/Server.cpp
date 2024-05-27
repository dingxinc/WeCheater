#include "Server.h"
#include <iostream>
#include "AsioIOServicePool.h"

Server::Server(boost::asio::io_context& io_context, short port):_io_context(io_context), _port(port),
_acceptor(io_context, tcp::endpoint(tcp::v4(),port))
{
	cout << "Server start success, listen on port : " << _port << endl;
	StartAccept();
}

Server::~Server() {
	cout << "Server destruct listen on port : " << _port << endl;
}

void Server::HandleAccept(shared_ptr<Session> new_session, const boost::system::error_code& error){
	if (!error) {
		new_session->Start();
		lock_guard<mutex> lock(_mutex);
		_sessions.insert(make_pair(new_session->GetUuid(), new_session));
	}
	else {
		cout << "session accept failed, error is " << error.what() << endl;
	}

	StartAccept();
}

void Server::StartAccept() {
	auto &io_context = AsioIOServicePool::GetInstance()->GetIOService();
	shared_ptr<Session> new_session = make_shared<Session>(io_context, this);
	_acceptor.async_accept(new_session->GetSocket(), std::bind(&Server::HandleAccept, this, new_session, placeholders::_1));
}

void Server::ClearSession(std::string uuid) {
	lock_guard<mutex> lock(_mutex);
	_sessions.erase(uuid);
}
