#pragma once
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "global.h"
#include "Singleton.h"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::GetVarifyReq;
using message::GetVarifyRsp;
using message::VarifyService;

/* RPC 的连接池 */
class RPConPool {
public:
	RPConPool(size_t poosize, std::string host, std::string port);
	~RPConPool();
	void Close();
	std::unique_ptr<VarifyService::Stub> getConnection();                  // 取连接
	void returnConnection(std::unique_ptr<VarifyService::Stub> context);   // 还连接

private:
	std::atomic<bool> b_stop_;   // 停止标志位
	size_t poolSize_;
	std::string host_;
	std::string port_;
	std::queue<std::unique_ptr<VarifyService::Stub>> connections_;
	std::condition_variable cond_;
	std::mutex mutex_;
};

class VarifyGrpcClient : public Singleton<VarifyGrpcClient>
{
	friend class Singleton<VarifyGrpcClient>;
public:
	GetVarifyRsp GetVarifyCode(std::string email) {  // 通过 email 获取一个 code 回包，用 GetVarifyRsp 接收
		ClientContext context;                       // 上下文环境
		GetVarifyRsp reply;                          // 回复
		GetVarifyReq request;                        // 请求
		request.set_email(email);                    // 把 email 设置进去

		auto stub = pool_->getConnection();          // 取连接
		Status status = stub->GetVarifyCode(&context, request, &reply);      // 远程调用获取验证码的服务，会回包一个状态
		if (status.ok()) {
			pool_->returnConnection(std::move(stub));// 还连接
			return reply;
		}
		else {
			pool_->returnConnection(std::move(stub));// 还连接
			reply.set_error(ErrorCodes::RPCFailed);
			return reply;
		}
	}

private:
	VarifyGrpcClient();
	// std::unique_ptr<VarifyService::Stub> stub_;      // 信使，只有通过它才能和别人通信
	std::unique_ptr<RPConPool> pool_;
};