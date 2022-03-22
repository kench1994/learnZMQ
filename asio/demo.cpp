#include "session.h"
#include "CharDelimParser.hpp"
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <utils/io_service_pool.hpp>

//1字节对齐
#pragma pack(1)
typedef struct tagAglorithm{
    tagAglorithm()
        : oper('+'), a(0.0), b(0.0), sum(0.0)
    {}
	tagAglorithm(char o, double aa, double bb, double cc)
		: oper('+'), a(aa), b(bb), sum(cc)
	{}
    //运算符 
    char oper;    
    //加数
    double a;
    double b;
    double sum;
}stAglorithm;
//取消对齐
#pragma pack()

int main()
{
    constexpr size_t ullContextLen = sizeof(stAglorithm);
    fprintf(stdout, "sizeof stAglorithm is %d\n", ullContextLen);
#ifdef _WIN32
	SetConsoleOutputCP(65001);
	CONSOLE_FONT_INFOEX info = { 0 }; 
    // 以下设置字体来支持中文显示。
	info.cbSize = sizeof(info);
	info.dwFontSize.Y = 16;
	info.FontWeight = FW_NORMAL;
	wcscpy(info.FaceName, L"Consolas");
	SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), NULL, &info);
#endif
    utils::io_service_pool::instance().run();
    auto spParser = std::make_shared<net::CharDelimParser>();
    //解包测试
    { 
        std::string ss = "Hello", ss2 = "World!";
        boost::shared_array<char> spszBuff(new char[2048]);
        memset(spszBuff.get(), 0x7e, 2048);
        memcpy_s(spszBuff.get() + 1, ss.length(), ss.c_str(), ss.length());
        memcpy_s(spszBuff.get() + 3 + ss.length(), ss2.length(), ss2.c_str(), ss2.length());
        spParser->parse(spszBuff, ss.length() + ss2.length() + 4);
    }
    auto spSession = std::make_shared<net::session>(spParser);
    auto spPeerInfo = std::make_shared<net::PeerInfo>("localhost", "7000");
    spSession->connect(spPeerInfo, [](const net::Reply& stReply){
        std::flush(std::cout);
        fprintf(stdout, "\n连接%s\n", stReply.nErrorCode ? "失败" : "成功");
    });
    while(1)
    {
        std::string strHolder, strErrInfo;
        std::cout << "输入表达式 (sample:1+2):";
        std::flush(std::cout);
        std::cin >> strHolder;

        std::vector<std::string>vTemp;
        for(const auto& itOP : {'+', '-', '*', '/'})
        {
            auto nPos = strHolder.find(itOP);
            if(nPos != std::string::npos)
            {
                vTemp.emplace_back(strHolder.substr(0, nPos));
                vTemp.emplace_back(strHolder.substr(nPos, 1));
                vTemp.emplace_back(strHolder.substr(nPos + 1));
                break;
            }
        }
        //boost::split(vTemp, strHolder, boost::is_any_of("+ - * /"), boost::token_compress_on);

        if(3 > vTemp.size())
        {
            std::cout << "非法表达式 " << std::endl;
            continue;
        }

        stAglorithm aglorithm;
        try{
            aglorithm.a = atof(vTemp[0].c_str());
            aglorithm.b = atof(vTemp[2].c_str());
            aglorithm.oper = vTemp[1][0];
        }catch(const std::exception& e)
        {
            std::cout << "非法表达式 " << std::endl;
            continue;
        }

        boost::shared_array<char>spszBuf(new char[sizeof(aglorithm) + 2]);
        memset(spszBuf.get(), '0x7e', sizeof(aglorithm) + 2);
        memcpy_s(spszBuf.get() + 1, sizeof(aglorithm), &aglorithm, sizeof(aglorithm));
        spSession->request(spszBuf.get(), 27, strErrInfo);
    }
    return 0;
}