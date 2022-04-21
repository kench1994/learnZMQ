#pragma once
#pragma once
#include "czmq.h"
#include <string>
#include <thread>
#include <memory>
class server {
public:
	server() = delete;
	explicit server(const char* listen_host_psz)
		: listen_host_(listen_host_psz)
	{

	}
	virtual ~server() {
		for (void* to_release_ptr : { zmq_sock_front_ptr_, zmq_sock_backend_ptr_, zmq_sock_worker_ptr_ })
		{
			if (!to_release_ptr)
				continue;
			zmq_close(to_release_ptr);
		}
		if (zmq_ctx_ptr_)
			zmq_ctx_destroy(zmq_ctx_ptr_);
	}

	bool initial() {
		zmq_ctx_ptr_ = zmq_ctx_new();
		if (!zmq_ctx_ptr_)
			return false;
		//  frontend套接字使用TCP和client通信
		zmq_sock_front_ptr_ = zmq_socket(zmq_ctx_ptr_, ZMQ_ROUTER);
		zmq_bind(zmq_sock_front_ptr_, listen_host_.c_str());
		//  backend套接字使用inproc和worker通信
		zmq_sock_backend_ptr_ = zmq_socket(zmq_ctx_ptr_, ZMQ_DEALER);
		zmq_bind(zmq_sock_backend_ptr_, "inproc://backend");
		//  server worker, 连接到backend
		zmq_sock_worker_ptr_ = zmq_socket(zmq_ctx_ptr_, ZMQ_DEALER);
		zmq_connect(zmq_sock_worker_ptr_, "inproc://backend");

		//  zthread_fork(ctx, server_worker, NULL);
		//  使用队列装置连接backend和frontend，我们本来可以这样做：
		//  zmq_device (ZMQ_QUEUE, frontend, backend);
		//  但这里我们会自己完成这个任务，这样可以方便调试。
		//  在frontend和backend间搬运消息

		work_trd_sptr_ = std::make_unique<std::thread>([this]() {
			//TODO: working sign
			for (;;) {
				work();
			}
		});
		work_trd_sptr_->detach();
	}
protected:
	void work() {
		zmq_pollitem_t items[] = {
			{ zmq_sock_front_ptr_, 0, ZMQ_POLLIN, 0 },
			{ zmq_sock_backend_ptr_,  0, ZMQ_POLLIN, 0 },
			{ zmq_sock_worker_ptr_, 0, ZMQ_POLLIN, 0 }
		};
		int nRet = zmq_poll(items, 3, -1);
		if (items[0].revents & ZMQ_POLLIN) {
			zmsg_t* msg = zmsg_recv(zmq_sock_front_ptr_);
			zmsg_send(&msg, zmq_sock_backend_ptr_);
		}
		if (items[1].revents & ZMQ_POLLIN) {
			zmsg_t* msg = zmsg_recv(zmq_sock_backend_ptr_);
			zmsg_send(&msg, zmq_sock_front_ptr_);
		}
		if (items[2].revents & ZMQ_POLLIN) {
			//  DEALER套接字将信封和消息内容一起返回给我们
			zmsg_t* msg = zmsg_recv(zmq_sock_worker_ptr_);
			//TODO:解耦事件处理
			zframe_print(zmsg_last(msg), "server recv");
			//fprintf(stdout, zmsg_dump());
			zframe_t* address = zmsg_pop(msg);
			zframe_t* content = zmsg_pop(msg);
			assert(content);
			zmsg_destroy(&msg);

			zframe_send(&address, zmq_sock_worker_ptr_, ZFRAME_REUSE + ZFRAME_MORE);
			zframe_send(&content, zmq_sock_worker_ptr_, ZFRAME_REUSE);

			zframe_destroy(&address);
			zframe_destroy(&content);
		}
	}
private:
	std::string listen_host_;
	void* zmq_ctx_ptr_;
	void* zmq_sock_front_ptr_, *zmq_sock_backend_ptr_, *zmq_sock_worker_ptr_;
	std::unique_ptr<std::thread> work_trd_sptr_;
};