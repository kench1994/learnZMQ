#pragma once
#include <string>
#include <boost/asio.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/function.hpp>


namespace utils
{
	class file_sink
		: public boost::enable_shared_from_this<file_sink>
	{
	public:
		enum error_code : int {

			reinit = -1,
			ok = 0,
			open_failed = 1
		};
		file_sink();
		~file_sink();

		int init(std::string &strFilePath);

		void destory();

		int begin_read(const boost::function<void(int, boost::shared_array<char>, unsigned int)>& fnOnData,\
			unsigned int offset = 0);

	protected:
		void on_read(const boost::system::error_code& ec, \
			boost::shared_array<char>spData, unsigned int uDataSize,\
			uint64_t offset, boost::function<void(int, boost::shared_array<char>, unsigned int)> fnOnData);

	private:
		std::string m_strFilePath;

		boost::shared_ptr<boost::asio::windows::random_access_handle> m_spAsyncFile;
	};
}