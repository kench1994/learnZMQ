//
//  异步C/S模型（DEALER-ROUTER）
// 
#include "czmq.h"
#include <thread>
#include <functional>
//  ---------------------------------------------------------------------
//  这是client端任务，它会连接至server，每秒发送一次请求，同时收集和打印应答消息。
//  我们会运行多个client端任务，使用随机的标识。
static void*
client_task(void* args)
{
    auto* ctx = zmq_ctx_new();
    void* client = zmq_socket(ctx, ZMQ_DEALER);
    //  设置随机标识，方便跟踪
    char identity[10];
    sprintf(identity, "%04X-%04X", randof(0x10000), randof(0x10000));
	zmq_setsockopt(client, ZMQ_IDENTITY, "A", 1);
    zmq_connect(client, "tcp://localhost:5570");
    zmq_pollitem_t items[] = { { client, 0, ZMQ_POLLIN, 0 } };
    int request_nbr = 0;
    while (1) {
        //  从poll中获取消息，每秒一次
        int centitick;
        for (centitick = 0; centitick < 100; centitick++) {
            zmq_poll(items, 1, 10 * ZMQ_POLL_MSEC);
            if (items[0].revents & ZMQ_POLLIN) {
                zmsg_t* msg = zmsg_recv(client);
                zframe_print(zmsg_last(msg), identity);
                zmsg_destroy(&msg);
            }
        }
        zstr_sendf(client, "request #%d", ++request_nbr);
    }
    zmq_term(&ctx);
    return NULL;
}
//  ---------------------------------------------------------------------
//  这是server端任务，它使用多线程机制将请求分发给多个worker，并正确返回应答信息。
//  一个worker只能处理一次请求，但client可以同时发送多个请求。
static void server_worker(void* args, void* ctx, void* pipe);
void* server_task(void* args)
{
    auto* ctx = zmq_ctx_new();
    //  frontend套接字使用TCP和client通信
    void* frontend = zmq_socket(ctx, ZMQ_ROUTER);
    zmq_bind(frontend, "tcp://*:5570");
    //  backend套接字使用inproc和worker通信
    void* backend = zmq_socket(ctx, ZMQ_DEALER);
    zmq_bind(backend, "inproc://backend");
    //  启动一个worker线程池，数量任意
    int thread_nbr;
    for (thread_nbr = 0; thread_nbr < 5; thread_nbr++)
    {
		auto* pTrd = new std::thread(std::bind(&server_worker, (void*)NULL, ctx, (void*)NULL));
		pTrd->detach();
    }
    //    zthread_fork(ctx, server_worker, NULL);
    //  使用队列装置连接backend和frontend，我们本来可以这样做：
    //      zmq_device (ZMQ_QUEUE, frontend, backend);
    //  但这里我们会自己完成这个任务，这样可以方便调试。
    //  在frontend和backend间搬运消息
    while (1) {
        zmq_pollitem_t items[] = {
            { frontend, 0, ZMQ_POLLIN, 0 },
            { backend,  0, ZMQ_POLLIN, 0 }
        };
        zmq_poll(items, 2, -1);
        if (items[0].revents & ZMQ_POLLIN) {
            zmsg_t* msg = zmsg_recv(frontend);
            //puts ("Request from client:");
            //zmsg_dump (msg);
            zmsg_send(&msg, backend);
        }
        if (items[1].revents & ZMQ_POLLIN) {
            zmsg_t* msg = zmsg_recv(backend);
            //puts ("Reply from worker:");
            //zmsg_dump (msg);
            zmsg_send(&msg, frontend);
        }
    }
    zmq_term(&ctx);
    return NULL;
}
//  接收一个请求，随机返回多条相同的文字，并在应答之间做随机的延迟。
//
static void
server_worker(void* args, void* ctx, void* pipe)
{
    void* worker = zmq_socket(ctx, ZMQ_DEALER);
    zmq_connect(worker, "inproc://backend");
    while (1) {
        //  DEALER套接字将信封和消息内容一起返回给我们
        zmsg_t* msg = zmsg_recv(worker);
        zframe_t* address = zmsg_pop(msg);
        zframe_t* content = zmsg_pop(msg);
        assert(content);
        zmsg_destroy(&msg);
        //  随机返回0至4条应答
        int reply, replies = randof(5);
        for (reply = 0; reply < replies; reply++) {
            //  暂停一段时间
            zclock_sleep(randof(1000) + 1);
            zframe_send(&address, worker, ZFRAME_REUSE + ZFRAME_MORE);
            zframe_send(&content, worker, ZFRAME_REUSE);
        }
        zframe_destroy(&address);
        zframe_destroy(&content);
    }
}
//  主程序用来启动多个client和一个server
//
int main(void)
{
    auto* ctx = zmq_ctx_new();
    
	auto* pTrd = new std::thread(std::bind(&server_task, (void*)NULL));
	pTrd->detach();

    for (int i = 0; i < 4; i++)
    {
        //zthread_new(ctx, client_task, NULL);
        auto* pTrd = new std::thread(std::bind(&client_task, (void*)NULL));
        if (i == 3)
            pTrd->join();
        else
            pTrd->detach();
    }
    //  运行5秒后退出
    zclock_sleep(5 * 1000);
    zmq_term(&ctx);
    return 0;
}