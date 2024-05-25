#pragma once
#include "global.h"

class HttpConnection;
using HttpHandler = std::function<void(std::shared_ptr<HttpConnection>)>;

class LogicSystem : public Singleton<LogicSystem>
{
	friend class Singleton<LogicSystem>;

public:
	~LogicSystem();
	bool HandleGet(std::string, std::shared_ptr<HttpConnection>);     // ���� Get ����
	void RegGet(std::string, HttpHandler handler);                    // ע�� Get ����
	bool HandlePost(std::string, std::shared_ptr<HttpConnection>);    // ���� post ����
	void RegPost(std::string, HttpHandler handler);                   // ע�� post ����

private:
	LogicSystem();
	std::map<std::string, HttpHandler> _post_handlers;
	std::map<std::string, HttpHandler> _get_handlers;
};

