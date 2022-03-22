#include "session.h"
#include <boost/bind.hpp>
namespace net
{

    void session::connect(std::shared_ptr<PeerInfo>spPeerInfo, reply_callback_t fnNotify)
    {
        //TODO:已连接状态检测

        m_spPeerInfo = std::move(spPeerInfo);

        auto* pIoService = utils::io_service_pool::instance().pick_io_service();
        boost::system::error_code ec;
        boost::asio::ip::tcp::resolver resolver(*pIoService);
        boost::asio::ip::tcp::resolver::query query(m_spPeerInfo->strHost.c_str(), m_spPeerInfo->strPort.c_str());
        auto resolver_result = resolver.resolve(query, ec);
        if (ec)
        {
            fnNotify({ec.value(), ec.message()});
            return;
        }

        auto spSocket = std::make_shared<boost::asio::ip::tcp::socket>(*pIoService);
        boost::asio::async_connect(*spSocket, resolver_result,\
            boost::bind(&session::on_connect, this,
                spSocket,
                boost::asio::placeholders::error,
                fnNotify
            )
        );
        return;
    }

    void session::on_connect(std::shared_ptr<boost::asio::ip::tcp::socket> spSocket,\
        const boost::system::error_code& ec, reply_callback_t fnNotify)
    {
        //连接失败
        if (ec)
        {

            if(fnNotify)
                fnNotify({ec.value(), ec.message()});

            m_anStatus.store(-1, std::memory_order::memory_order_relaxed);

            return;
        }
        
        m_spSocket = std::move(spSocket);
        
        //TODO:连接状态机

        //begin recive
        do_recv();
        
        m_anStatus.store(0, std::memory_order::memory_order_relaxed);

        if(fnNotify)
            fnNotify({0, ""});
    }

    void session::do_recv()
    {
        //prepare space
        boost::shared_array<char>spszBuffer(new char[4096]);
        memset(spszBuffer.get(), 0, 4096);

        //async read
        m_spSocket->async_read_some(boost::asio::buffer(spszBuffer.get(), 4096), \
            boost::bind(&session::on_recv, this, \
                spszBuffer, \
                boost::asio::placeholders::bytes_transferred, \
                boost::asio::placeholders::error
            )
        );
    }

    void session::on_recv(boost::shared_array<char>spszBuff, size_t ullRecvSize, const boost::system::error_code& ec)
    {
        //TODO:解析工作放到消费者队列
        fprintf(stdout, "on recv\n");

        if (!ec)
            return;
        
        //TODO: notify user
    }

    int session::request(const char* pszSendBuf, size_t ullSendLen, std::string& strErrInfo)
    {
        //TODO:check conn status
        if(0 != m_anStatus.load(std::memory_order::memory_order_relaxed))
        {
            strErrInfo = "session unconn";
            return -1;
        }


        m_spSocket->async_send(boost::asio::buffer(pszSendBuf, ullSendLen),
            boost::bind(&session::on_send, 
                this,
                boost::asio::placeholders::bytes_transferred,
                boost::asio::placeholders::error
            )
        );

        return 0;
    }

    void session::on_send(size_t ullSendSize, const boost::system::error_code& ec)
    {
        if (!ec)
        {
            fprintf(stdout, "on send len %d\n", ullSendSize);
            return;
        }
        //TODO:
    }
}