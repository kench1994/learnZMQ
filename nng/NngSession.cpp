#include "QmtNngSession.h"
#include "../../utils/slave_node_helper/instance_log.h"
#include "QmtGlobalTool.h"
using namespace iudge;
using namespace broker;
QmtNngSession::QmtNngSession()
	: m_nConnStatus(0), m_pAcceptAio(nullptr), m_uThisAddress((unsigned int)this),
	m_spIO(nullptr), m_spStrand(nullptr), m_aullIdGen(0)
{

}

QmtNngSession::~QmtNngSession()
{

}

int QmtNngSession::Init(boost::shared_ptr<boost::asio::io_service>spIO, std::string& strErrInfo)
{
	if (!m_spIO)
		m_spIO = spIO;
	if (!m_spStrand)
		m_spStrand = boost::make_shared<boost::asio::io_service::strand>(*spIO);
	utils::Read_Lock rLock(m_rwConnStauts);
	if (1 == m_nConnStatus)
	{
		strErrInfo = "Already Conncted Status";
		return 0;
	}
	int nRet = nng_pair0_open(&m_nngSocket);	//创建套接字
	if (0 == nRet)
	{
		auto fnConnStatusNotify = [](nng_pipe p, int e, void* pArg) {
			__LOG(qmt_implement::g_spUserMessage, "OnConnStatus:%d", e);
			auto pOwner = static_cast<QmtNngSession*>(pArg);
			pOwner->OnConnStatus(e);
		};
		auto pThisAddress = GetThisAddress();
		nng_pipe_notify(m_nngSocket, NNG_PIPE_EV_REM_POST, fnConnStatusNotify, static_cast<void*>(this));	//在管道从套接字中移除后 调用回调
		nng_pipe_notify(m_nngSocket, NNG_PIPE_EV_ADD_POST, fnConnStatusNotify, static_cast<void*>(this));	//在管道完全添加到套接字后 调用回调
	}
	return nRet;
}

int QmtNngSession::Connect(const std::string& strUrl, std::string& strErrInfo)
{
	int ret = 0;
	if ((ret = nng_dial(m_nngSocket, strUrl.data(), NULL, 0)) < 0)	//创建并启动拨号程序：发起连接，成功建立连接后，创建一个管道，将其添加到套接字，然后等待该管道关闭。
		strErrInfo.assign("nng_dial:").append(std::to_string(ret));
	else
		DoRecv();
	return ret;
}

int QmtNngSession::Request(const char* pszDtsSrc, size_t szDtsLen, fntUserNotify fnNotify, \
	std::string& strErrInfo, int nExpired /*= 10 * 1000*/)
{
	return Send(MsgType::RequestType, pszDtsSrc, szDtsLen, fnNotify, strErrInfo, nExpired);
}


int QmtNngSession::Tweet(const char* pszDtsSrc, size_t szDtsLen, fntUserNotify fnNotify, \
	std::string& strErrInfo, int nExpired /*= 10 * 1000*/)
{
	return Send(MsgType::TweetType, pszDtsSrc, szDtsLen, fnNotify, strErrInfo, nExpired);
}

void QmtNngSession::OnSend()
{
	m_spStrand->post([spThis = shared_from_this()]() {
		//弹出已发送数据
		spThis->m_vSendQueue.pop_front();
		if (spThis->m_vSendQueue.empty())
			return;
		auto spEvent = spThis->m_vSendQueue.front();
		nng_send_aio(spThis->m_nngSocket, spEvent->pAio);
	});
}

void QmtNngSession::DoRecv()
{
	int nRet = nng_aio_alloc(&m_pAcceptAio, [](void* pArg) {	//为异步 I/O 操作分配一个句柄，并将指向它的指针存储在m_pAcceptAio中
																//句柄使用回调进行初始化，当相关的异步操作完成时将执行该回调。它将使用参数pArg调用
		try
		{
			//todo 重发策略
			if (!pArg)
				throw(std::runtime_error("nng recv aio has null handler"));
			auto pSession = (QmtNngSession*)*(unsigned int*)pArg;
			auto aio_status = nng_aio_result(pSession->m_pAcceptAio);	//返回与句柄相关联的操作的结果。如果操作成功，则返回 0。否则返回非零错误代码。
			bool bMsgReady = false;
			switch (aio_status)
			{
			case 0:
				bMsgReady = true;
				break;
			case NNG_ETIMEDOUT:	//超时
				break;
			default:
			{
				//触发onError?
				break;
			}
			}
			if (bMsgReady)
			{
				auto pMsg = nng_aio_get_msg(pSession->m_pAcceptAio);	//获取存储在aio 中的任何消息作为成功接收的结果
				boost::shared_ptr<nng_msg> spNngMsg(pMsg, [](nng_msg* pMsg) {
					nng_msg_free(pMsg);
				});

				pSession->GetStrand()->dispatch(boost::bind(&QmtNngSession::OnMessage, pSession, spNngMsg));
			}
			//继续下次接收
			pSession->DoRecv();
		}
		catch (...)
		{
			//通知到onError
			assert("crashed");
		}
	}, static_cast<void*>(GetThisAddress()));
	//todo 失败尝试重试
	if (0 != nRet)
		return;
	nng_recv_aio(m_nngSocket, m_pAcceptAio);	//套接字收到消息后 将其存储在aio中 nng_aio_set_msg()  执行m_pAcceptAio异步io的回调函数  在这种情况下，nng_aio_result()将返回零。
}

void QmtNngSession::OnMessage(boost::shared_ptr<nng_msg>spNngMsg)
{
	auto nMsgLen = nng_msg_len(spNngMsg.get());
	std::string strMsg = boost::string_view((const char*)nng_msg_body(spNngMsg.get()), nMsgLen).to_string();
	auto szBuff = (const char*)nng_msg_body(spNngMsg.get());
	auto nHeaderLen = 2 + 4 + 8;
	//boost::shared_array<char> spszBuffer(new char[szHeaderLen]);
	//memcpy_s(spszBuffer.get(), szHeaderLen, strMsg.data(), szHeaderLen);

	/*uint16_t nMsgType = *(uint16_t*)(spszBuffer.get());
	uint64_t nID = *(uint64_t*)(spszBuffer.get() + 2);
	uint32_t nLen = *(uint32_t*)(spszBuffer.get() + 10);*/

	uint16_t nMsgType = 0;// *(uint16_t*)(spszBuffer.get());
	uint64_t nID = 0;
	uint32_t nLen = 0;

	memcpy_s(&nMsgType, sizeof(nMsgType), szBuff, sizeof(nMsgType));
	memcpy_s(&nID, sizeof(nID), szBuff + sizeof(nMsgType), sizeof(nID));
	memcpy_s(&nLen, sizeof(nLen), szBuff + sizeof(nMsgType) + sizeof(nID), sizeof(nLen));

	switch (nMsgType)
	{
	case MsgType::RequestType:
	{
		//请求
		/*utils::Write_Lock wLock(m_rwRequsetWait);
		auto itEvent = m_mapRequest.find(nID);
		if (itEvent != m_mapRequest.end())
		{
			itEvent->second->fnNotify(0, spNngMsg, strMsg);
			m_mapRequest.erase(itEvent);
		}
		wLock.unlock();*/
		break;
	}
	case MsgType::ResponseType:
	{
		//回复
		//std::string strBody = strMsg.substr(szHeaderLen,strMsg.size());
		utils::Write_Lock wLock(m_rwRequsetWait);
		auto itEvent = m_mapRequest.find(nID);
		boost::shared_ptr<Event> spEvent(nullptr);
		if (itEvent != m_mapRequest.end())
		{
			spEvent = itEvent->second;
			m_mapRequest.erase(itEvent);
		}
		wLock.unlock();
		if (spEvent)
		{
			m_spStrand->post([spNngMsg, spEvent]() {
				if (!spEvent || spEvent->fnNotify)
					spEvent->fnNotify(0, spNngMsg, "");
			});
		}
		break;
	}
	case MsgType::TweetType:
	{
		//推送
		m_spStrand->post([spNngMsg, spThis = shared_from_this()]() {
			spThis->m_fnOnTweet(spNngMsg);
		});
		break;
	}
	default:
		break;
	}
}

void QmtNngSession::OnConnStatus(int nStatus)
{
	utils::Write_Lock wLock(m_rwConnStauts);
	m_nConnStatus = nStatus;
	wLock.unlock();
	if (1 == nStatus)
		return;
	OnError(nStatus, "conn status error");
}


void QmtNngSession::OnError(const int nErrorCode, const std::string& strErrInfo)
{
	utils::Write_Lock wLock(m_rwConnStauts);
	auto mapRegist = m_mapRequest;
	m_mapRequest.clear();
	wLock.unlock();
	for (const auto& iter : mapRegist)
	{
		if (iter.second->fnNotify)
			iter.second->fnNotify(nErrorCode, nullptr, strErrInfo);
	}
}

unsigned int* QmtNngSession::GetThisAddress()
{
	return &m_uThisAddress;
}

boost::shared_ptr<boost::asio::io_service::strand> QmtNngSession::GetStrand()
{
	return m_spStrand;
}

boost::shared_ptr<boost::asio::io_service> QmtNngSession::GetIo()
{
	return m_spIO;
}

void iudge::broker::QmtNngSession::SetOnTweet(boost::function<void(boost::shared_ptr<nng_msg>)>fnOnTweet)
{
	m_fnOnTweet = fnOnTweet;
}


int QmtNngSession::Send(MsgType type, const char* pszDtsSrc, size_t szDtsLen, fntUserNotify fnNotify, \
	std::string& strErrInfo, int nExpired)
{
	utils::Read_Lock rLock(m_rwConnStauts);
	auto nConnStatus = m_nConnStatus;
	if (1 != nConnStatus)
	{
		strErrInfo.assign("Conn Status Not Correct:").append(std::to_string(nConnStatus));
		return 10010;
	}
	rLock.unlock();

	auto spEvent = boost::make_shared<Event>();
	spEvent->ullVisitor = *GetThisAddress();
	auto uuId = m_aullIdGen.fetch_add(1, std::memory_order::memory_order_relaxed);
	spEvent->ullReqId = uuId;
	spEvent->fnNotify = fnNotify;

	utils::Write_Lock wLock(m_rwRequsetWait);
	m_mapRequest[spEvent->ullReqId] = spEvent;
	wLock.unlock();

	int ret = 0;
	nng_msg* msg;
	if ((ret = nng_msg_alloc(&msg, 0)) != 0)
	{
		strErrInfo.assign("nng_msg_alloc:").append(std::to_string(ret));
		return ret;
	}

	auto nHeaderLen = 2 + 4 + 8;
	boost::shared_array<char> spszBuffer(new char[nHeaderLen]);
	memset(spszBuffer.get(), '\0', nHeaderLen);
	//*(uint16_t *)spszBuffer.get() = iudge::MsgType::Request;
	//*(uint64_t *)(spszBuffer.get() + 2) = uuId;
	//(uint32_t *)(spszBuffer.get() + 10) = szDtsLen;
	//消息类型、事件ID、正文长度memcpy到数组
	memcpy_s(spszBuffer.get(), sizeof(unsigned short), &type, sizeof(unsigned short));
	memcpy_s(spszBuffer.get() + sizeof(unsigned short), sizeof(unsigned long long), &uuId, sizeof(unsigned long long));
	memcpy_s(spszBuffer.get() + sizeof(unsigned short) + sizeof(unsigned long long), sizeof(unsigned int), &szDtsLen, sizeof(unsigned int));


	if ((ret = nng_msg_append(msg, spszBuffer.get(), nHeaderLen)) != 0)
	{
		strErrInfo.assign("nng_msg_append:").append(std::to_string(ret));
		return ret;
	}


	//包体追加到msg
	if ((ret = nng_msg_append(msg, pszDtsSrc, szDtsLen)) != 0)
	{
		strErrInfo.assign("nng_msg_append:").append(std::to_string(ret));
		return ret;
	}



	if ((ret = nng_aio_alloc(&spEvent->pAio, [](void* pEvent) {//为异步 I/O 操作分配一个句柄，并将指向它的指针存储在pAio中
																//句柄使用回调进行初始化，当相关的异步操作完成时将执行该回调。它将使用参数pArg调用
		auto pRegistEvent = (QmtNngSession::Event*)pEvent;
		auto pOwner = (QmtNngSession*)(unsigned int)pRegistEvent->ullVisitor;
		auto aio_status = nng_aio_result(pRegistEvent->pAio);	//返回与句柄相关联的操作的结果。如果操作成功，则返回 0。否则返回非零错误代码。
		bool bSuccess = false;
		switch (aio_status)
		{
		case 0:
		{
			if (0 < pRegistEvent->nTimeoutMillseconds)
			{
				auto nResumeMillseconds = pRegistEvent->nTimeoutMillseconds - std::chrono::duration_cast<std::chrono::microseconds>(\
					std::chrono::system_clock::now() - pRegistEvent->tpAppCall).count();
				pRegistEvent->spTimer = boost::make_shared<boost::asio::deadline_timer>(*pOwner->GetIo());
				pRegistEvent->spTimer->expires_from_now(boost::posix_time::milliseconds(nResumeMillseconds));
				pRegistEvent->spTimer->async_wait([uuId = pRegistEvent->ullReqId, pOwner](const boost::system::error_code& ec) {
					if (ec)
						return;
					utils::Write_Lock wLock(pOwner->m_rwRequsetWait);
					auto itF = pOwner->m_mapRequest.find(uuId);
					if (itF == pOwner->m_mapRequest.end())
						return;
					auto spEvent = itF->second;
					pOwner->m_mapRequest.erase(itF);
					wLock.unlock();

					if (!spEvent->fnNotify)
						return;
					pOwner->GetStrand()->dispatch([spEvent]() {
						spEvent->fnNotify(10010, nullptr, "任务响应超时");
					});
				});
			}
			bSuccess = true;
			break;
		}
		case NNG_ETIMEDOUT:
			break;
		default:
		{
			pOwner->GetStrand()->dispatch([pOwner, aio_status]() {
				pOwner->OnError(aio_status,
					std::string("aio status error:")
					.append(std::to_string(aio_status))
				);
			});
			break;
		}
		}
		nng_aio_free(pRegistEvent->pAio);

		//直接发送下一个
		pOwner->OnSend();

		if (bSuccess)
			return;
		//todo work in trd
		//发送失败通知
		if (pRegistEvent->spTimer)
			pRegistEvent->spTimer->cancel();
		if (pRegistEvent->fnNotify)
		{
			//todo notify no thread pool
			pRegistEvent->fnNotify(
				aio_status,
				nullptr,
				std::string("nng_aio_result ret:").append(std::to_string(aio_status))
			);
		}
	}, static_cast<void*>(spEvent.get()))) != 0)
	{
		//todo
		nng_msg_free(msg);
		return ret;
	}

	nng_aio_set_msg(spEvent->pAio, msg);	//设置将用于异步发送操作的消息

	if (0 < nExpired)
	{
		spEvent->nTimeoutMillseconds = nExpired;
		nng_aio_set_timeout(spEvent->pAio, nExpired);
		spEvent->tpAppCall = std::chrono::system_clock::now();
	}


	m_spStrand->post([spEvent, spThis = shared_from_this()]() {
		bool bSendNow = spThis->m_vSendQueue.empty();
		spThis->m_vSendQueue.push_back(spEvent);
		if (!bSendNow)
			return;
		nng_send_aio(spThis->m_nngSocket, spEvent->pAio);
	});
	return ret;
}

