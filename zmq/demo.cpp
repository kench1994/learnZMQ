#include "server.h"
#include "client.h"


int main(void)
{
	server s("tcp://*:5570");
	s.initial();
	zclock_sleep(100);

	for (int i = 0; i < 3; i++)
	{
		auto client_ptr = new client(i);
		client_ptr->connect("tcp://localhost:5570");
		client_ptr->send();
	}
	getchar();
	return 0;
}