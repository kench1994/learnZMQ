#pragma once
#include <mutex>
#include <memory>
#include <string>
#include <boost/shared_array.hpp>
namespace net
{

    class IParser
    {
        public:
            IParser() {}
            virtual ~IParser() {}
            
            virtual int parse(boost::shared_array<char> spszIncomm, unsigned int) = 0;

        private:
    };
} // namespace net
