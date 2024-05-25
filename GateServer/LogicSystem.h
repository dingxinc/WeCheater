#pragma once
#include "global.h"

class HttpConnection;
using HttpHandler = std::function<void(std::shared_ptr<HttpConnection>)>;

class LogicSystem : public Singleton<LogicSystem>
{
	friend class Singleton<LogicSystem>;

public:
	~LogicSystem();
	bool HandleGet(std::string, std::shared_ptr<HttpConnection>);     // 处理 Get 请求
	void RegGet(std::string, HttpHandler handler);                    // 注册 Get 请求
	bool HandlePost(std::string, std::shared_ptr<HttpConnection>);    // 处理 post 请求
	void RegPost(std::string, HttpHandler handler);                   // 注册 post 请求

private:
	LogicSystem();
	std::map<std::string, HttpHandler> _post_handlers;
	std::map<std::string, HttpHandler> _get_handlers;
};

