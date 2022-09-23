#include "incommer.h"
#include <boost/shared_array.hpp>

incommer::incommer() {

}
incommer::incommer(const fntOnFlood& fnNotifyData)
	: m_fnOnDataIncomm(fnNotifyData)
{

}

incommer::~incommer()
{
}

void incommer::setSocket(std::shared_ptr<boost::asio::ip::tcp::socket>&& spSocket)
{
	m_spSocket = spSocket;
}

void incommer::doRecv()
{
	unsigned int uBufferSize = 4096; 
	boost::shared_array<char>spszBuffer(new char[uBufferSize]);
	memset(spszBuffer.get(), 0, uBufferSize);

	//boost::asio::bind_executor(m_spSocket->get_io_service(),

	m_spSocket->async_read_some(boost::asio::buffer(spszBuffer.get(), uBufferSize),
		boost::bind(&incommer::onRecv, this,
			spszBuffer,
			boost::asio::placeholders::bytes_transferred,
			boost::asio::placeholders::error
		)
	);
}

void incommer::onRecv( \
	boost::shared_array<char>spszBuff, \
	unsigned int uRecvSize, \
	const boost::system::error_code& ec
) 
{
	if (m_fnOnDataIncomm) {

		m_fnOnDataIncomm(spszBuff, uRecvSize, ec.value(), ec.message().c_str());
	}

	//TODO: 连接异常处理
	if (ec)
		return;

	doRecv();
}
