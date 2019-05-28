#include <iostream>
#include "little_rpc.h"

using namespace little_rpc;

int main()
{
	RpcServer rpc_server;
	rpc_server.start("tcp://*:5000");

	return 0;
}