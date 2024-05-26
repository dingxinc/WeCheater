#include "LogicSystem.h"
#include "HttpConnection.h"
#include "VarifyGrpcClient.h"
#include "RedisMgr.h"
#include "MysqlMgr.h"

LogicSystem::~LogicSystem()
{
}

bool LogicSystem::HandleGet(std::string path, std::shared_ptr<HttpConnection> conn)
{
	if (_get_handlers.find(path) == _get_handlers.end()) {  // û�ҵ�
		return false;
	}
	// �ҵ��ˣ�����
	_get_handlers[path](conn);
	return true;
}

void LogicSystem::RegGet(std::string url, HttpHandler handler)
{
	_get_handlers.insert(std::make_pair(url, handler));
}

bool LogicSystem::HandlePost(std::string path, std::shared_ptr<HttpConnection> conn)
{
	if (_post_handlers.find(path) == _post_handlers.end()) {  // û�ҵ�
		return false;
	}
	// �ҵ��ˣ�����
	_post_handlers[path](conn);
	return true;
}

void LogicSystem::RegPost(std::string url, HttpHandler handler)
{
	_post_handlers.insert(std::make_pair(url, handler));
}

LogicSystem::LogicSystem()
{
	// ע�� Get ����
	RegGet("/get_test", [](std::shared_ptr<HttpConnection> connection) {
		beast::ostream(connection->_response.body()) << "receive get_test req";
		int i = 0;
		for (auto& elem : connection->_get_params) {
			i++;
			beast::ostream(connection->_response.body()) << "param" << i << " key is " << elem.first;
			beast::ostream(connection->_response.body()) << ", " << " value is " << elem.second << std::endl;
		}
	});

	// ��ȡ��֤�� post ����
	RegPost("/get_varifycode", [](std::shared_ptr<HttpConnection> connection) {
		// ���յ�������� body �������תΪ string
		auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
		std::cout << "receive body is " << body_str << std::endl;
		connection->_response.set(http::field::content_type, "text/json");   // ���߿ͻ��˻ظ��������� json ��ʽ��
		Json::Value root;     // Ҫ���ظ��Է��Ľڵ�
		Json::Reader reader;
		Json::Value src_root; // ��Դ�ڵ�
		bool parser_success = reader.parse(body_str, src_root);  // ���� body_str����������� src_root��
		if (!parser_success) { // ����ʧ��
			std::cout << "Failed to parser JSON data!" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string jsonstr = root.toStyledString();  // ���л�
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		if (!src_root.isMember("email")) {  // key ������
			std::cout << "Failed to parser JSON data!" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string jsonstr = root.toStyledString();  // ���л�
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		// �����ɹ� �� key ����
		auto email = src_root["email"].asString();   // �ͻ��˻ᷢ��һ�� email ����
		GetVarifyRsp rsp = VarifyGrpcClient::GetInstance()->GetVarifyCode(email);    // ͨ�� grpc ����Զ����֤�����
		std::cout << "email is " << email << std::endl;
		root["error"] = rsp.error();  // û�д���
		root["email"] = src_root["email"];  // ���ͻ��˷���������Ҳ���ظ��ͻ���
		std::string jsonstr = root.toStyledString();  // ���л�
		beast::ostream(connection->_response.body()) << jsonstr;
		return true;
	});

	// �û�ע�� post ����
	RegPost("/user_register", [](std::shared_ptr<HttpConnection> connection) {
		auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
		std::cout << "receive body is " << body_str << std::endl;
		connection->_response.set(http::field::content_type, "text/json");
		Json::Value root;
		Json::Reader reader;
		Json::Value src_root;
		bool parse_success = reader.parse(body_str, src_root);
		if (!parse_success) {
			std::cout << "Failed to parse JSON data!" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		auto email = src_root["email"].asString();
		auto name = src_root["user"].asString();
		auto pwd = src_root["passwd"].asString();
		auto confirm = src_root["confirm"].asString();

		if (pwd != confirm) {
			std::cout << "password err " << std::endl;
			root["error"] = ErrorCodes::PasswdErr;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		//�Ȳ���redis��email��Ӧ����֤���Ƿ����
		std::string  varify_code;
		bool b_get_varify = RedisMgr::GetInstance()->Get(CODEPREFIX + src_root["email"].asString(), varify_code);
		if (!b_get_varify) {
			std::cout << " get varify code expired" << std::endl;
			root["error"] = ErrorCodes::VarifyExpired;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		if (varify_code != src_root["varifycode"].asString()) {
			std::cout << " varify code error" << std::endl;
			root["error"] = ErrorCodes::VarifyCodeErr;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		////����redis����
		//bool b_usr_exist = RedisMgr::GetInstance()->ExistsKey(src_root["user"].asString());
		//if (b_usr_exist) {
		//	std::cout << " user exist" << std::endl;
		//	root["error"] = ErrorCodes::UserExist;
		//	std::string jsonstr = root.toStyledString();
		//	beast::ostream(connection->_response.body()) << jsonstr;
		//	return true;
		//}

		//�������ݿ��ж��û��Ƿ����
		int uid = MysqlMgr::GetInstance()->RegUser(name, email, pwd);
		if (uid == 0 || uid == -1) {
			std::cout << " user or email exist" << std::endl;
			root["error"] = ErrorCodes::UserExist;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		root["error"] = 0;
		root["uid"] = uid;
		root["email"] = email;
		root["user"] = name;
		root["passwd"] = pwd;
		root["confirm"] = confirm;
		root["varifycode"] = src_root["varifycode"].asString();
		std::string jsonstr = root.toStyledString();
		beast::ostream(connection->_response.body()) << jsonstr;
		return true;
	});
}