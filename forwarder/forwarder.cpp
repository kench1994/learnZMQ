#include "forwarder.h"
#include <boost/bind.hpp>
#include <functional>
#include <iostream>
forwarder::forwarder(unsigned int uPort)
	: m_spListener(std::make_unique<listener>(uPort))
	, m_spInId(std::make_unique<utils::LckFreeIncId<unsigned int>>(0))
	, m_spToId(std::make_unique<utils::LckFreeIncId<unsigned int>>(0))

{
}

forwarder::~forwarder()
{
}

int forwarder::beginListen()
{
	return m_spListener->work(std::bind(&forwarder::onListen, this, std::placeholders::_1));
}

unsigned int forwarder::getPort()
{
	return m_spListener ? m_spListener->getPort() : 0;
}

int forwarder::addBackend(const std::string& strHost, \
	const std::string& strPort, \
	unsigned int uChkInter /*= 0\ */)
{
	auto id = m_spToId->increase();
	auto spBackend = std::make_shared<backend>(strHost, strPort, uChkInter);

	std::unique_lock<std::mutex> lck(m_mtxIncomms);
	m_mapBackends[id] = spBackend;
	lck.unlock();

	//try conn
	//连接可能耗时
	boost::system::error_code ec;
	return spBackend->connRemote([](const int& nErrorCode, const char* pszErrInfo) {
		std::cout << nErrorCode << std::endl;
	});
}

void forwarder::onListen(std::shared_ptr<boost::asio::ip::tcp::socket> spSocket)
{
	auto id = m_spInId->increase();
	auto spIncommer = std::make_shared<incommer>();
	spIncommer->setSocket(std::move(spSocket));

	std::unique_lock<std::mutex> lck(m_mtxIncomms);
	m_mapIncomms[id] = spIncommer;
	lck.unlock();

	//drive incommer work
	spIncommer->doRecv();
}

void forwarder::onIncommDts(boost::shared_array<char> spszBuffer,\
	unsigned int uRecvSize, const int& nErrorCode, const char* pszErrInfo)
{
	//遍历转发给健康的backend
	std::unique_lock<std::mutex> lck(m_mtxBackends);
	auto m = m_mapBackends;
	lck.unlock();

	for (const auto& iter : m) {
		//iter->second->
	}
}

void forwarder::onBackendStatus()
{

}
