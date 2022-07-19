//#include <iostream>
//#include <string>
//#include <vector>
//#include <algorithm>
#include "RC4.h"
#include <array>
#include <memory>
#include <chrono>
#include <thread>
#include <iostream>
#include <unordered_map>
#include <boost/lexical_cast.hpp>
#include <boost/shared_array.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
//#include <Poco/Net/FTPClientSession.h>

#pragma comment(lib, "bcrypt")
#define MAX_BUFF_SIZE 2048
#define MAX_TOKEN_SIZE 50



inline std::string uuid()
{
	static boost::uuids::random_generator generator;

	boost::uuids::uuid uuid_gen = generator();
	return boost::lexical_cast<std::string>(uuid_gen);
}

typedef struct tipray_encrypt_t {

	int setcmdenc;
	int usetoken;
	char token[MAX_TOKEN_SIZE];
} tipray_encrypt_t;

inline void Tipray_Encrypt(const char* v_szSrc, char* v_szDest)
{
	int iSrcLen = (int)strlen(v_szSrc);
	if (iSrcLen <= 0 || iSrcLen >= MAX_BUFF_SIZE - 3)
		return;

	const char* szEncKey = "tipray";
	RC4EncryptStr(v_szDest + 2, v_szSrc, iSrcLen, szEncKey, (int)strlen(szEncKey));
	*(v_szDest + 0) = '2';
	*(v_szDest + 1) = 1 + '0';
}

int main(int argc, char **argv)
{
	//std::string host = "192.168.152.144";
	//std::string username = "server12345";
	//std::string password = "server12345";
	//Poco::UInt16 port = 52004;

	//std::string ss = "This is a test";
	//char szResult[20]{ '\0' };
	//Tipray_Encrypt(ss.data(), szResult);


	//std::unordered_map<unsigned int, std::shared_ptr<Poco::Net::FTPClientSession>> hmapSessions;
	//
	//for (unsigned int i = 0; i < 1; i++)
	//{
	//	auto spFtpSession = std::make_shared<Poco::Net::FTPClientSession>(host, port, "", "");
	//	spFtpSession->login(username, password);

	//	while(!spFtpSession->isLoggedIn()){
	//		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	//	}

	//	hmapSessions.emplace(i, std::move(spFtpSession));
	//}

	//int nRet = 0;
	//std::string strResponse;

	//auto spSession = hmapSessions.at(0);

	//spSession->setFileType(Poco::Net::FTPClientSession::TYPE_BINARY);
	//spSession->setPassive(true);

	//100M���ļ� 100 * 1024KB
	//ÿ�η���64K
	//��Ҫ��1600�η���
	std::array<boost::shared_array<char>, 16> fileBlocks;
	for (unsigned int i = 0; i < 16; i++)
	{
		boost::shared_array<char> spBuffer(new char[65536]);
		memset(spBuffer.get(), i + 1, 65536);
		fileBlocks[i] = std::move(spBuffer);
	}
	//std::ostream& ostr = spSession->beginUpload("ToDesk_Lite.exe");


	//spSession->endUpload();
	//spSession->close();
	getchar();
	//get file list
	//std::string str;
	//std::vector<std::string> filelist;
	//std::istream &ftpin = ftpsession.beginList("/SobeyRes/aaa/");//����Ŀ¼���ļ��б�
	//while (ftpin >> str)
	//{//�洢�ļ��б�
	//	filelist.push_back(str);
	//}
	//ftpsession.endList();//�ر�Ŀ¼��������

	//					 //download all file to local device
	//for (size_t i = 0; i != filelist.size(); i++)
	//{
	//	std::cout << "cur file is: " << filelist[i] << "...\n";

	//	std::ofstream file;
	//	file.open(filelist[i].c_str());
	//	//����ÿ���ļ����洢���ַ���content��
	//	Poco::StreamCopier::copyStream(ftpsession.beginDownload(filelist[i].c_str()), file);

	//	ftpsession.endDownload();//�ر���������
	//}
	//ftpsession.close();//�Ͽ�FTP

	return 0;
}