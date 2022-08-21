#pragma once
#include <list>
#include <string>
#include <chrono>
#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>
#include <nng/protocol/pair0/pair.h>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/utility/string_view.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "../../utils/public/read_write_mutex.h"
//#include "QmtUserDefine.h"

namespace iudge
{
	namespace broker
	{
		typedef boost::shared_mutex READ_WRITE_MUTEX;
		typedef boost::shared_lock<READ_WRITE_MUTEX> Read_Lock;
		typedef boost::unique_lock<READ_WRITE_MUTEX> Write_Lock;


		/*
		todo on session error
		*/

		using fntUserNotify = boost::function<void(int, boost::shared_ptr<nng_msg>, const std::string&)>;
		class QmtNngSession
			: public boost::enable_shared_from_this<QmtNngSession>
		{
		public:
			enum MsgType
			{
				RequestType = 0,	//请求
				ResponseType,		//响应
				TweetType			//推送
			};

			typedef struct tagEvent {
				std::chrono::system_clock::time_point tpAppCall;
				//std::chrono::system_clock::time_point tpAioReply;
				boost::shared_ptr<boost::asio::deadline_timer>spTimer;
				fntUserNotify fnNotify;
				nng_aio* pAio;
				unsigned long long ullReqId = 0;
				unsigned int ullVisitor = 0;
				int nTimeoutMillseconds = 0;
			}Event;

			QmtNngSession();
			~QmtNngSession();


			int Init(boost::shared_ptr<boost::asio::io_service>spIO, std::string& strErrInfo);

			int Connect(const std::string& strUrl, std::string& strErrInfo);

			int Request(const char* pszDtsSrc, size_t szDtsLen, fntUserNotify fnNotify, std::string& strErrInfo, int nExpired = 10 * 1000);

			int Tweet(const char* pszDtsSrc, size_t szDtsLen, fntUserNotify fnNotify, std::string& strErrInfo, int nExpired = 10 * 1000);

			void SetOnTweet(boost::function<void(boost::shared_ptr<nng_msg>)>fnOnTweet);
		protected:
			int Send(MsgType type, const char* pszDtsSrc, size_t szDtsLen, fntUserNotify fnNotify, \
				std::string& strErrInfo, int nExpired = 10 * 1000);

			void OnSend();

			void DoRecv();

			void OnMessage(boost::shared_ptr<nng_msg>spNngMsg);

			void OnConnStatus(int nStatus);

			void OnError(const int nErrorCode, const std::string& strErrInfo);

			unsigned int* GetThisAddress();

			boost::shared_ptr<boost::asio::io_service::strand> GetStrand();

			boost::shared_ptr<boost::asio::io_service> GetIo();
		private:
			//把this地址放在此处
			unsigned int m_uThisAddress;

			boost::shared_ptr<boost::asio::io_service>m_spIO;
			boost::shared_ptr<boost::asio::io_service::strand>m_spStrand;

			std::list<boost::shared_ptr<Event>>m_vSendQueue;
			std::atomic<unsigned long long> m_aullIdGen;
			//aio接收任务
			nng_aio* m_pAcceptAio;

			utils::READ_WRITE_MUTEX m_rwConnStauts;
			nng_socket m_nngSocket;
			int m_nConnStatus;

			utils::READ_WRITE_MUTEX m_rwRequsetWait;
			std::map<unsigned long long, boost::shared_ptr<Event>>m_mapRequest;

			boost::function<void(boost::shared_ptr<nng_msg>)> m_fnOnTweet;
		};
	}
}