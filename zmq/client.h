#pragma once
#include "czmq.h"
#include <string>
#include <thread>
#include <memory>
class client {
public:
	client() = delete;

	explicit client(int client_id)
	{
		client_id_ = client_id;
		zmq_ctx_ptr_ = zmq_ctx_new();
		zmq_sock_ptr_ = zmq_socket(zmq_ctx_ptr_, ZMQ_DEALER);
		auto client_id_str = std::to_string(client_id_);
		zmq_setsockopt(zmq_sock_ptr_, ZMQ_IDENTITY, client_id_str.c_str(), client_id_str.length());

		work_trd_sptr = std::make_unique<std::thread>([this]() {
			for (;;) {
				do_recv();
			}
		});
		work_trd_sptr->detach();
	}
	virtual ~client() {
		if (zmq_sock_ptr_)
			zmq_close(zmq_sock_ptr_);
		if (zmq_ctx_ptr_)
			zmq_ctx_destroy(zmq_ctx_ptr_);
	}

	int connect(const char* peer_host_psz) {
		return zmq_connect(zmq_sock_ptr_, peer_host_psz);
	}

	
	void send() {
		zstr_sendf(zmq_sock_ptr_, "request #%d", client_id_);
	}
protected:
	//TODO:单线程管理多socket
	int do_recv() {
		zmq_pollitem_t items[] = { { zmq_sock_ptr_, 0, ZMQ_POLLIN, 0 } };
		/*
		n>0: n个socket上都有事件发生
		n=0: 超时发生
		n=-1: 失败
		*/
		int poll_ret = zmq_poll(items, 1, -1);
		switch (poll_ret)
		{
			case 0:
				//任务超时
				return poll_ret;
			case -1:
				//任务失败?
				return poll_ret;
			default:
				break;
		}
		if (items[0].revents & ZMQ_POLLIN) {
			zmsg_t* msg = zmsg_recv(zmq_sock_ptr_);
			zframe_print(zmsg_last(msg), "client recv");
			zmsg_destroy(&msg);
		}
		//如果有trd_pool的话就投递,,没有的话就直接一个线程循环
	}
private:
	int client_id_;
	void* zmq_ctx_ptr_;
	void* zmq_sock_ptr_;

	std::unique_ptr<std::thread> work_trd_sptr;
};