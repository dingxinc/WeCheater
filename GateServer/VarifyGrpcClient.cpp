#include "VarifyGrpcClient.h"
#include "ConfigMgr.h"

RPConPool::RPConPool(size_t poolsize, std::string host, std::string port) : poolSize_(poolsize), host_(host), port_(port), b_stop_(false)
{
	for (size_t i = 0; i < poolSize_; ++i) {
		std::shared_ptr<Channel> channel = grpc::CreateChannel(host + ":" + port, grpc::InsecureChannelCredentials());
		// stub_ = VarifyService::NewStub(channel);    // 信使相当于电话员， channel 相当于电话，通过 channel 与服务器通信
		connections_.push(VarifyService::NewStub(channel));  // grpc 在使用的时候，就关注两个东西， channel 和 stub
	}
}

RPConPool::~RPConPool()
{
	std::lock_guard<std::mutex> lock(mutex_);
	Close();
	while (!connections_.empty()) {
		connections_.pop();
	}
}

void RPConPool::Close()
{
	b_stop_ = true;
	cond_.notify_all();
}

std::unique_ptr<VarifyService::Stub> RPConPool::getConnection()
{
	std::unique_lock<std::mutex> lock(mutex_);
	cond_.wait(lock, [this]() {
		if (b_stop_) {
			return true;
		}
		return !connections_.empty();  // 如果队列为空，返回 false, 此时线程先挂起，再解锁，线程不会往下走了，直到有别的人唤醒它，notify_one 或者 notify_all
	});

	/* 线程往下走，证明队列不为空，可以取了 */
	if (b_stop_) return nullptr;
	auto context = std::move(connections_.front());
	connections_.pop();
	return context;
}

void RPConPool::returnConnection(std::unique_ptr<VarifyService::Stub> context)
{
	std::lock_guard<std::mutex> lock(mutex_);
	if (b_stop_) return;
	connections_.push(std::move(context));
	cond_.notify_one();                        // 还了连接，证明连接池里有连接了，可以通知线程去取了
}

VarifyGrpcClient::VarifyGrpcClient() {
	auto& gCfgMgr = ConfigMgr::GetInstance();
	std::string host = gCfgMgr["VarifyServer"]["Host"];
	std::string port = gCfgMgr["VarifyServer"]["Port"];
	pool_.reset(new RPConPool(5, host, port));  // 初始化连接池
}