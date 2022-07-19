#include "file_sink.h"
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

namespace utils
{
	static const size_t BUFFER_SIZE = 65536;

	file_sink::file_sink()
	{
	}

	file_sink::~file_sink()
	{
	}

	int file_sink::init(std::string& strFilePath)
	{
		m_strFilePath = strFilePath;

		//文件句柄
		auto hFile = CreateFile(strFilePath.c_str(),
			FILE_READ_DATA,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_OVERLAPPED,
			NULL
		);
		if (INVALID_HANDLE_VALUE == hFile)
			return error_code::open_failed;
		//异步文件句柄
		//m_spAsyncFile = boost::make_shared<boost::asio::windows::random_access_handle>(*spIO, hFile);
		return 0;
	}

	void file_sink::destory()
	{
		if (!m_spAsyncFile)
			return;
		{
			if (m_spAsyncFile->is_open())
				m_spAsyncFile->close();
			m_spAsyncFile.reset();
		}
	}

	int file_sink::begin_read(const boost::function<void(int, boost::shared_array<char>, unsigned int)>& fnOnData, unsigned int offset)
	{
		boost::shared_array<char>spBuffer(new char[65536]);
		memset(spBuffer.get(), 0, 65536);

		m_spAsyncFile->async_read_some_at(
			offset, boost::asio::buffer(spBuffer.get(), BUFFER_SIZE),
			boost::bind(&file_sink::on_read, shared_from_this(),
				boost::asio::placeholders::error, spBuffer, boost::asio::placeholders::bytes_transferred,\
				offset, fnOnData)
		);
		return 0;
	}

	void file_sink::on_read(const boost::system::error_code& ec,
		boost::shared_array<char> spData, unsigned int uDataSize,
		uint64_t offset, boost::function<void(int, boost::shared_array<char>, unsigned int)> fnOnData)
	{
		if (ec)
		{
			if (fnOnData)
				fnOnData(ec.value(), nullptr, 0);
			return;
		}

		if (fnOnData)
			fnOnData(0, spData, uDataSize);

		offset += uDataSize;

		begin_read(fnOnData, offset);
	}
}