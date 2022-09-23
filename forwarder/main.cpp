// ReverseProxy.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "forwarder.h"
#include "Utils/utils/io_service_pool.hpp"
#include <boost/asio.hpp>

int main()
{
	
	utils::io_service_pool::instance().run();

	auto spForwarder = std::make_shared<forwarder>(80);
	spForwarder->beginListen();
	spForwarder->addBackend("192.168.5.246", "20182");
	//spAcceptor->open(endpoint.protocol());
	//spAcceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	//spAcceptor->bind(endpoint);
	//spAcceptor->listen();
	//spAcceptor->async_accept()
	getchar();
	return 0;
}

