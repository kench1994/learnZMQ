#pragma once
#include <memory>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/shared_array.hpp>
class incommer
	: public std::enable_shared_from_this<incommer>
{
protected:
	using fntOnInComm = boost::function<void(boost::shared_array<char>, \
		unsigned int, const boost::system::error_code&)>;
public:
	using fntOnFlood = std::function<void(boost::shared_array<char>, \
		unsigned int, const int&, const char*)>;

	incommer();

	incommer(const fntOnFlood& fnNotifyData);

	~incommer();

	void setSocket(std::shared_ptr<boost::asio::ip::tcp::socket>&& spSocket);

	void doRecv();
protected:
	void onRecv(boost::shared_array<char>spszBuff,\
		unsigned int uRecvSize, \
		const boost::system::error_code& ec
	);
private:
	std::shared_ptr<boost::asio::ip::tcp::socket> m_spSocket;

	fntOnFlood m_fnOnDataIncomm;
};

