#include "backend.h"
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include "Utils/utils/io_service_pool.hpp"

backend::backend(const std::string& strHost, const std::string& strPort, unsigned int uChkInter /*= 0*/)
	: m_spBackendInfo(std::make_shared<stBackendInfo>(strHost, strPort, uChkInter))
	, m_auStaus(_enConnStatus::unconn)
{

}

backend::~backend()
{
}

int backend::connRemote(const fntOnConnStatus &fnOnStatus)
{
	auto *pIoService = IOS.pick_io_service();
	boost::asio::ip::tcp::resolver resolver(*pIoService);
	boost::asio::ip::tcp::resolver::query query(\
		m_spBackendInfo->strHost.c_str(), m_spBackendInfo->strPort.c_str()
	);

	boost::system::error_code ec;
	auto resolver_result = resolver.resolve(query, ec);
	if (ec) {
		//TODO:通知失败
		fnOnStatus(ec.value(), ec.message().c_str());
		return -1;
	}
	m_fnOnConnStatus = fnOnStatus;
	auto spSocket = std::make_shared<socket>(resolver.get_io_service());
	boost::asio::async_connect(*spSocket, resolver_result,
		std::bind(&backend::onConned, this, spSocket, std::placeholders::_1)
	);
	//异步 conn 任务提交成功
	return 0;
}

int backend::chkAndAddChain(const boost::shared_array<char>& spszBuffer, unsigned int uSendSize)
{
	if (_enConnStatus::connected != m_auStaus.load())
		return -1;
	std::unique_lock<std::mutex> lck(m_mtxQueue);
	//if()
	//如果队列 == 1 触发发送
	//m_spSocket->async_send()
}

_enConnStatus backend::getStatus()
{
	auto s = m_auStaus.load();
	return s;
}

void backend::onConned(std::shared_ptr<socket>spSocket, const boost::system::error_code ec)
{
	if (m_fnOnConnStatus)
		m_fnOnConnStatus(ec.value(), ec.message().c_str());

	if (ec.value()) {
		m_auStaus.store(_enConnStatus::unconn);
		return;
	}

	m_auStaus.store(_enConnStatus::connected);
	m_spSocket = std::move(spSocket);
}
