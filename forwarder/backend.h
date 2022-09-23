#pragma once
#include "Conf.h"
#include <queue>
#include <mutex>
#include <boost/asio.hpp>
#include <boost/shared_array.hpp>
//�м̷���ʵ��
//�൱�ڿͻ���
//class relayer

class backend
{
public:
	//����״̬�仯�¼�
	using fntOnConnStatus = std::function<void(const int&, const char* )>;
	//backend server��Ϣ
	typedef struct tagBackend {
		tagBackend(const std::string& host, const std::string& port, unsigned int inter)
			: strHost(host), strPort(port)
		{}
		std::string strHost;
		std::string strPort;
	}stBackendInfo;

	backend() = delete;
	backend(const std::string& strHost, const std::string& strPort, unsigned int uChkInter = 0);
	~backend();

	int connRemote(const fntOnConnStatus &fnOnStatus);

	int chkAndAddChain(const boost::shared_array<char>&, unsigned int uSendSize);

	_enConnStatus getStatus();
protected:

	using socket = boost::asio::ip::tcp::socket;

	void onConned(std::shared_ptr<socket>spSocket, const boost::system::error_code ec);

	void doSend() {}

	void onSend() {}
private:
	std::atomic<_enConnStatus> m_auStaus;

	fntOnConnStatus m_fnOnConnStatus;

	std::shared_ptr<socket> m_spSocket;

	std::shared_ptr<stBackendInfo> m_spBackendInfo;

	//���Ͷ���
	std::mutex m_mtxQueue;
	std::queue<std::shared_ptr<void>> m_Queue;
};

