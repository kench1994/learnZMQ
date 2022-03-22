#pragma once
#include "IPraser.h"
#include <algorithm>
#include <iostream>
namespace net
{
	class CharDelimParser
		: public virtual IParser
	{
		public:
			enum class ParseStatus : unsigned int{
				wait_begin,
				wait_end
			};

			CharDelimParser()
				: m_chDelimiter(0x7e),\
				  m_spBuffer(std::make_shared<std::vector<char>>())
			{}
			virtual ~CharDelimParser(){}

			int parse(boost::shared_array<char> spszIncomm, unsigned int uIncommSize) override
			{
				//TODO: 

				std::unique_lock<std::mutex> lock(m_mtxBuf);
				//添加到缓存,继续解析
				if(uIncommSize)
				{
					auto prio_len = m_spBuffer->size();
					m_spBuffer->resize(prio_len + uIncommSize);
					std::copy(spszIncomm.get(), spszIncomm.get() + uIncommSize, m_spBuffer->begin() + prio_len);
				}
				while(1)
				{
					auto curr_len = m_spBuffer->size();
					//长度不足
					if(2 > curr_len)
						return 0;

					//tcp流异常 
					char chDelimit = m_spBuffer->at(0);
					if (m_chDelimiter != chDelimit)
						return -1;
					
					//帧头已确定,找帧尾 
					std::vector<char>::iterator itF = std::find(m_spBuffer->begin() + 1, m_spBuffer->end(), m_chDelimiter);
					if (itF == m_spBuffer->end())
					{
						fprintf(stdout, "unfind\n");
						return 1;
					}
				
					//找到帧尾,截出一帧  
					//加一字节find跳过的距离 
					auto distance = std::distance(m_spBuffer->begin(), itF) + 1;
					auto spFrame = boost::shared_array<char>(new char[distance]);
					memcpy_s(spFrame.get(), distance, m_spBuffer->data(), distance);
					//TODO:通知使用者 
					fprintf(stdout, "get a frame len %d ->%s\n", distance, std::string(spFrame.get(), distance).c_str());

					//截断已消化的数据
					m_spBuffer->erase(m_spBuffer->begin(), itF + 1);


					//可能还有长度,进入下个while循环 
				}
				return 2;
			}

		private:
			char m_chDelimiter;
			
			std::mutex m_mtxBuf;
			std::shared_ptr<std::vector<char>> m_spBuffer;


			//ParseStatus m_enParseStatus;
	};
}