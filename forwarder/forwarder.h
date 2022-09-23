#pragma once
#include "backend.h"
#include "incommer.h"
#include "listener.h"
#include "LckFreeIncId.h"
#include <mutex>
#include <unordered_map>
class forwarder
{
public:
	forwarder() = delete;
	explicit forwarder(unsigned int uPort);
	~forwarder();

	//�����˿ڼ���
	int beginListen();


	unsigned int getPort();


	//Ϊ��� forwarder ��� backend
	int addBackend(const std::string& strHost, \
		const std::string& strPort, \
		unsigned int uChkInter = 0 \
	);


protected:
	void onListen(std::shared_ptr<boost::asio::ip::tcp::socket> spSocket);

	//��������
	void onIncommDts(boost::shared_array<char>, unsigned int, const int&, const char*);

	void onBackendStatus();
private:
	std::unique_ptr<listener> m_spListener;

	//���η�����
	std::mutex m_mtxBackends;
	std::unordered_map<unsigned int, std::shared_ptr<backend>> m_mapBackends;
	std::unique_ptr<utils::LckFreeIncId<unsigned int>> m_spToId;

	//���ûỰ�б�
	std::mutex m_mtxIncomms;
	std::unordered_map<unsigned int, std::shared_ptr<incommer>> m_mapIncomms;
	std::unique_ptr<utils::LckFreeIncId<unsigned int>> m_spInId;

};

