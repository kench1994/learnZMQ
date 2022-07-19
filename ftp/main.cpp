//#include <iostream>
//#include <string>
//#include <vector>
//#include <algorithm>
#include <Poco/File.h>
#include <Poco/Net/FTPClientSession.h>
#include <Poco/StreamCopier.h>
#include "RC4.h"
#include <memory>
#include <chrono>
#include <thread>
#include <iostream>
#include <unordered_map>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#pragma comment(lib, "bcrypt")
#define MAX_BUFF_SIZE 2048
#define MAX_TOKEN_SIZE 50

using Poco::File;
using Poco::StreamCopier;
using Poco::Net::FTPClientSession;


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
	std::string host = "192.168.152.144";
	std::string username = "server12345";
	std::string password = "server12345";
	Poco::UInt16 port = 52004;

	std::string ss = "This is a test";
	char szResult[20]{ '\0' };
	Tipray_Encrypt(ss.data(), szResult);


	std::unordered_map<unsigned int, std::shared_ptr<FTPClientSession>> hmapSessions;
	
	for (unsigned int i = 0; i < 1; i++)
	{
		auto spFtpSession = std::make_shared<FTPClientSession>(host, port, "", "");
		spFtpSession->login(username, password);

		while(!spFtpSession->isLoggedIn()){
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		hmapSessions.emplace(i, std::move(spFtpSession));
	}

	int nRet = 0;
	std::string strResponse;

	auto spSession = hmapSessions.at(0);

	nRet = hmapSessions.at(0)->sendCommand("TYPE", "I", strResponse);
	


	//nRet = spSession->sendCommand("PASV", strResponse);
	//std::vector<std::string> vDtsStr;
	//boost::algorithm::split(vDtsStr, strResponse, boost::is_any_of(",()"));
	//unsigned int uDtsPort = 0;
	//if (8 == vDtsStr.size())
	//	uDtsPort += (boost::lexical_cast<unsigned int>(vDtsStr[5]) << 8 + boost::lexical_cast<unsigned int>(vDtsStr[6]));
	spSession->setPassive(true);
	std::ostream& ostr = spSession->beginUpload("ToDesk_Lite.exe");
	FILE* fp = fopen("C:\\Users\\kench\\Downloads\\ToDesk_Lite.exe", "rb");
	fseek(fp, 0, SEEK_END);
	//获取文件大小;
	auto fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	long resumeSize = 0;
	for (long sendSize = 0; sendSize < fileSize; ) {
		//一次最多发送1024
		resumeSize = fileSize - sendSize;
		if (resumeSize > 1024)
			resumeSize = 1024;
		//准备空间
		char* szBuffer = new char[resumeSize];
		fread(szBuffer, resumeSize, 1, fp);
		ostr.write(szBuffer, resumeSize);
		ostr.flush();
		delete szBuffer;
		sendSize += resumeSize;
	}
	spSession->endUpload();
	spSession->close();
	getchar();
	//get file list
	//std::string str;
	//std::vector<std::string> filelist;
	//std::istream &ftpin = ftpsession.beginList("/SobeyRes/aaa/");//下载目录中文件列表
	//while (ftpin >> str)
	//{//存储文件列表
	//	filelist.push_back(str);
	//}
	//ftpsession.endList();//关闭目录下载连接

	//					 //download all file to local device
	//for (size_t i = 0; i != filelist.size(); i++)
	//{
	//	std::cout << "cur file is: " << filelist[i] << "...\n";

	//	std::ofstream file;
	//	file.open(filelist[i].c_str());
	//	//下载每个文件并存储到字符串content中
	//	Poco::StreamCopier::copyStream(ftpsession.beginDownload(filelist[i].c_str()), file);

	//	ftpsession.endDownload();//关闭下载连接
	//}
	//ftpsession.close();//断开FTP

	return 0;
}