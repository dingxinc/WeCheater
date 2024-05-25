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

class VarifyGrpcClient : public Singleton<VarifyGrpcClient>
{
	friend class Singleton<VarifyGrpcClient>;
public:
	GetVarifyRsp GetVarifyCode(std::string email) {  // 通过 email 获取一个 code 回包，用 GetVarifyRsp 接收
		ClientContext context;                       // 上下文环境
		GetVarifyRsp reply;                          // 回复
		GetVarifyReq request;                        // 请求
		request.set_email(email);                    // 把 email 设置进去

		Status status = stub_->GetVarifyCode(&context, request, &reply);      // 远程调用获取验证码的服务，会回包一个状态
		if (status.ok()) {
			return reply;
		}
		else {
			reply.set_error(ErrorCodes::RPCFailed);
			return reply;
		}
	}

private:
	VarifyGrpcClient() {
		std::shared_ptr<Channel> channel = grpc::CreateChannel("0.0.0.0:50051", grpc::InsecureChannelCredentials());
		stub_ = VarifyService::NewStub(channel);    // 信使相当于电话员， channel 相当于电话，通过 channel 与服务器通信
	}
	std::unique_ptr<VarifyService::Stub> stub_;      // 信使，只有通过它才能和别人通信
};

