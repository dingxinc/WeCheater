#pragma once
#include <boost/asio.hpp>
#include "Session.h"
#include <memory.h>
#include <map>
#include <mutex>
using namespace std;
using boost::asio::ip::tcp;

class Server
{
public:
	Server(boost::asio::io_context& io_context, short port);
	~Server();
	void ClearSession(std::string);

private:
	void HandleAccept(shared_ptr<Session>, const boost::system::error_code & error);
	void StartAccept();
	boost::asio::io_context &_io_context;
	short _port;
	tcp::acceptor _acceptor;
	std::map<std::string, shared_ptr<Session>> _sessions;
	std::mutex _mutex;
};

