#pragma once
#include <boost/asio.hpp>
#include <functional>
#include <atomic>
class listener
{
public:
	using socket = boost::asio::ip::tcp::socket;
	using fntOnIncommer = std::function<void(const std::shared_ptr<socket>&)>;
	listener() = delete;

	explicit listener(const unsigned int uPort);

	~listener();

	int work(const fntOnIncommer& fnNotify);

	void stop();

	unsigned int getPort();

protected:
	int beginListen(const fntOnIncommer& fnNotify);
	
	void onListen(\
		std::shared_ptr<boost::asio::ip::tcp::socket> spSocket, 
		const boost::system::error_code& ec, \
		const fntOnIncommer fnOnListener
	);

private:
	std::shared_ptr<boost::asio::ip::tcp::acceptor>m_spAcceptor;

	unsigned int m_uPort;

	std::atomic<bool> m_abOnListen;
};

