#pragma once
#include "IPraser.h"
#include <mutex>
#include <memory>
#include <functional>
#include <boost/asio.hpp>
#include <boost/asio/strand.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_array.hpp>
#include <Utils/io_service_pool.hpp>
namespace net
{
    typedef struct tagPeerInfo {
        tagPeerInfo(const char* pszHost, const char* pszPort)
            : strHost(pszHost), strPort(pszPort) {}
        std::string strHost;
        std::string strPort;
    }PeerInfo;


    typedef struct tagReply{
        tagReply() : nErrorCode(0) {}
        tagReply(int code, std::string errinfo = "", std::shared_ptr<void> data = nullptr)
            : nErrorCode(code), strErrInfo(std::move(errinfo)), spData(std::move(data))
        {}
        //错误代码
        int nErrorCode;
        //错误信息
        std::string strErrInfo;
        //业务数据
        std::shared_ptr<void> spData;
    }Reply;

    typedef std::function<void(const Reply&)> reply_callback_t;

    class session
        :   private boost::noncopyable
    {
        public:

            session() = delete;

            //todo:can I check derive relation
			template <typename T>
            session(std::shared_ptr<T> spParser)
                : m_spParser(std::move(std::static_pointer_cast<IParser>(spParser))),
                  m_anStatus(-1)
            {}


            virtual ~session() {}

            void connect(std::shared_ptr<PeerInfo>spPeerInfo, reply_callback_t fnNotify);

            //应用层二进制化好之后提交到此接口
            int request(const char* pszSendBuf, size_t ullSendLen, std::string& strErrInfo);
    protected:

            void do_recv();
            
            void on_recv(boost::shared_array<char>spszBuff, size_t ullRecvSize, const boost::system::error_code& ec);

            void on_send(size_t ullSendSize, const boost::system::error_code& ec);

            void on_connect(std::shared_ptr<boost::asio::ip::tcp::socket> spSocket,\
              const boost::system::error_code& ec, reply_callback_t fnNotify);
        private:
            std::atomic<int> m_anStatus;

            std::mutex m_mtxPeerInfo;
            std::shared_ptr<PeerInfo> m_spPeerInfo;
            
            std::mutex m_mtxSocket;
            std::shared_ptr<boost::asio::ip::tcp::socket> m_spSocket;

            std::shared_ptr<IParser> m_spParser;
    };
}