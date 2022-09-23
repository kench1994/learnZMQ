#pragma once
#include "backend.h"
#include "incommer.h"
#include "listener.h"
#include "LckFreeIncId.h"
#include <mutex>
#include <unordered_map>
class forwarder
{
public:
	forwarder() = delete;
	explicit forwarder(unsigned int uPort);
	~forwarder();

	//开启端口监听
	int beginListen();


	unsigned int getPort();


	//为这个 forwarder 添加 backend
	int addBackend(const std::string& strHost, \
		const std::string& strPort, \
		unsigned int uChkInter = 0 \
	);


protected:
	void onListen(std::shared_ptr<boost::asio::ip::tcp::socket> spSocket);

	//数据流量
	void onIncommDts(boost::shared_array<char>, unsigned int, const int&, const char*);

	void onBackendStatus();
private:
	std::unique_ptr<listener> m_spListener;

	//下游服务器
	std::mutex m_mtxBackends;
	std::unordered_map<unsigned int, std::shared_ptr<backend>> m_mapBackends;
	std::unique_ptr<utils::LckFreeIncId<unsigned int>> m_spToId;

	//来访会话列表
	std::mutex m_mtxIncomms;
	std::unordered_map<unsigned int, std::shared_ptr<incommer>> m_mapIncomms;
	std::unique_ptr<utils::LckFreeIncId<unsigned int>> m_spInId;

};

