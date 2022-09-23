#include "listener.h"
#include "Utils/utils/io_service_pool.hpp"


listener::listener(const unsigned int uPort)
	: m_abOnListen(false)
	, m_uPort(uPort)
{
}


listener::~listener()
{
}

int listener::work(const fntOnIncommer& fnNotify)
{
	if (m_spAcceptor || m_abOnListen.load(std::memory_order::memory_order_relaxed))
		return -1;

	boost::asio::ip::tcp::endpoint ep(boost::asio::ip::tcp::v4(), m_uPort);

	m_spAcceptor = std::make_shared<boost::asio::ip::tcp::acceptor>(\
		*utils::io_service_pool::instance().pick_io_service(), \
		ep, false
	);
	if (!m_spAcceptor)
		return -1;

	beginListen(fnNotify);
	return 0;
}


void listener::stop()
{
	m_spAcceptor->close();
	m_abOnListen.store(false, std::memory_order::memory_order_relaxed);
}


int listener::beginListen(const fntOnIncommer& fnNotify)
{
	try {
		if (!m_spAcceptor || !m_spAcceptor->is_open())
			return false;

		m_abOnListen.store(true, std::memory_order::memory_order_relaxed);

		auto spSocket = std::make_shared<boost::asio::ip::tcp::socket>(m_spAcceptor->get_io_service());
		m_spAcceptor->async_accept(*spSocket, \
			std::bind(&listener::onListen, this, spSocket, \
				std::placeholders::_1, fnNotify)
		);
	} catch (const std::exception&) {
		//TODO:LOG
		//e.what();
		return false;
	}
	return true;
}

void listener::onListen(\
	std::shared_ptr<boost::asio::ip::tcp::socket> spSocket,
	const boost::system::error_code& ec, \
	const fntOnIncommer fnOnListener
)
{
	if (ec) {
		fnOnListener(nullptr);
		return;
	}
	fnOnListener(spSocket);

	//continue listen
	beginListen(fnOnListener);
}

unsigned int listener::getPort()
{
	return m_uPort;
}
