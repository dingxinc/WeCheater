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

/* RPC �����ӳ� */
class RPConPool {
public:
	RPConPool(size_t poosize, std::string host, std::string port);
	~RPConPool();
	void Close();
	std::unique_ptr<VarifyService::Stub> getConnection();                  // ȡ����
	void returnConnection(std::unique_ptr<VarifyService::Stub> context);   // ������

private:
	std::atomic<bool> b_stop_;   // ֹͣ��־λ
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
	GetVarifyRsp GetVarifyCode(std::string email) {  // ͨ�� email ��ȡһ�� code �ذ����� GetVarifyRsp ����
		ClientContext context;                       // �����Ļ���
		GetVarifyRsp reply;                          // �ظ�
		GetVarifyReq request;                        // ����
		request.set_email(email);                    // �� email ���ý�ȥ

		auto stub = pool_->getConnection();          // ȡ����
		Status status = stub->GetVarifyCode(&context, request, &reply);      // Զ�̵��û�ȡ��֤��ķ��񣬻�ذ�һ��״̬
		if (status.ok()) {
			pool_->returnConnection(std::move(stub));// ������
			return reply;
		}
		else {
			pool_->returnConnection(std::move(stub));// ������
			reply.set_error(ErrorCodes::RPCFailed);
			return reply;
		}
	}

private:
	VarifyGrpcClient();
	// std::unique_ptr<VarifyService::Stub> stub_;      // ��ʹ��ֻ��ͨ�������ܺͱ���ͨ��
	std::unique_ptr<RPConPool> pool_;
};