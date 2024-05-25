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
	GetVarifyRsp GetVarifyCode(std::string email) {  // ͨ�� email ��ȡһ�� code �ذ����� GetVarifyRsp ����
		ClientContext context;                       // �����Ļ���
		GetVarifyRsp reply;                          // �ظ�
		GetVarifyReq request;                        // ����
		request.set_email(email);                    // �� email ���ý�ȥ

		Status status = stub_->GetVarifyCode(&context, request, &reply);      // Զ�̵��û�ȡ��֤��ķ��񣬻�ذ�һ��״̬
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
		stub_ = VarifyService::NewStub(channel);    // ��ʹ�൱�ڵ绰Ա�� channel �൱�ڵ绰��ͨ�� channel �������ͨ��
	}
	std::unique_ptr<VarifyService::Stub> stub_;      // ��ʹ��ֻ��ͨ�������ܺͱ���ͨ��
};

