#include "LogicSystem.h"
#include "HttpConnection.h"
#include "VarifyGrpcClient.h"

LogicSystem::~LogicSystem()
{
}

bool LogicSystem::HandleGet(std::string path, std::shared_ptr<HttpConnection> conn)
{
	if (_get_handlers.find(path) == _get_handlers.end()) {  // 没找到
		return false;
	}
	// 找到了，处理
	_get_handlers[path](conn);
	return true;
}

void LogicSystem::RegGet(std::string url, HttpHandler handler)
{
	_get_handlers.insert(std::make_pair(url, handler));
}

bool LogicSystem::HandlePost(std::string path, std::shared_ptr<HttpConnection> conn)
{
	if (_post_handlers.find(path) == _post_handlers.end()) {  // 没找到
		return false;
	}
	// 找到了，处理
	_post_handlers[path](conn);
	return true;
}

void LogicSystem::RegPost(std::string url, HttpHandler handler)
{
	_post_handlers.insert(std::make_pair(url, handler));
}

LogicSystem::LogicSystem()
{
	// 注册 Get 请求
	RegGet("/get_test", [](std::shared_ptr<HttpConnection> connection) {
		beast::ostream(connection->_response.body()) << "receive get_test req";
		int i = 0;
		for (auto& elem : connection->_get_params) {
			i++;
			beast::ostream(connection->_response.body()) << "param" << i << " key is " << elem.first;
			beast::ostream(connection->_response.body()) << ", " << " value is " << elem.second << std::endl;
		}
	});

	// 注册 Post 请求
	RegPost("/get_varifycode", [](std::shared_ptr<HttpConnection> connection) {
		// 把收到的请求的 body 里的数据转为 string
		auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
		std::cout << "receive body is " << body_str << std::endl;
		connection->_response.set(http::field::content_type, "text/json");   // 告诉客户端回复的数据是 json 格式的
		Json::Value root;     // 要返回给对方的节点
		Json::Reader reader;
		Json::Value src_root; // 来源节点
		bool parser_success = reader.parse(body_str, src_root);  // 解析 body_str，解析完放在 src_root中
		if (!parser_success) { // 解析失败
			std::cout << "Failed to parser JSON data!" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string jsonstr = root.toStyledString();  // 序列化
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		if (!src_root.isMember("email")) {  // key 不存在
			std::cout << "Failed to parser JSON data!" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string jsonstr = root.toStyledString();  // 序列化
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		// 解析成功 且 key 存在
		auto email = src_root["email"].asString();   // 客户端会发送一个 email 过来
		GetVarifyRsp rsp = VarifyGrpcClient::GetInstance()->GetVarifyCode(email);    // 通过 grpc 调用远程验证码服务
		std::cout << "email is " << email << std::endl;
		root["error"] = rsp.error();  // 没有错误
		root["email"] = src_root["email"];  // 将客户端发过来邮箱也发回给客户端
		std::string jsonstr = root.toStyledString();  // 序列化
		beast::ostream(connection->_response.body()) << jsonstr;
		return true;
	});
}