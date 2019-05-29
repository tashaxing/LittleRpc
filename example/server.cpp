#include <iostream>
#include <unordered_set>
#include "little_rpc.h"

using namespace little_rpc;

class MyServerCallback : public RpcServerCallback
{
public:
	void bind_server(RpcServer* server)
	{
		// save server and set self as server callback
		m_server = server;
		m_server->set_callback(this);
	}

	void publish(const std::string& data)
	{
		// publish to all connected clients
		
		for (std::string conn_id : m_clients)
			m_server->send(conn_id.c_str(), data.c_str());
	}

	virtual void on_rpc_request(const char *conn_id, const char *req_data)
	{
		// here you can dispatch request with self defined request method
		std::cout << "receive rpc request, conn_id: " << conn_id 
				  << " req_data: " << req_data << std::endl;

		// send response
		std::string rsp_str = "response to " + std::string(conn_id) + " content: " + req_data;
		m_server->send(conn_id, rsp_str.c_str());

		// you can save the conn_id and send notification msg to specific client at any time
		m_clients.insert(conn_id);
	}

	virtual void on_close(const char *conn_id)
	{
		// TODO: for heartbeat
		std::cout << conn_id << " client connection closed" << std::endl;
		m_clients.erase(conn_id);
 	}

private:
	RpcServer* m_server;
	std::unordered_set<std::string> m_clients;
};

int main()
{
	const char* addr = "tcp://*:5000";

	// bind server and callback
	RpcServer rpc_server;
	MyServerCallback my_server; // the read server handler
	my_server.bind_server(&rpc_server);

	// start server in seperate thread
	std::thread server_thread(&RpcServer::start, &rpc_server, addr);
	std::cout << "rpc server is listening on " << addr << std::endl;

	// test send notification
	std::this_thread::sleep_for(std::chrono::milliseconds(8000));
	my_server.publish("welcome to rpc server!");
	std::cout << "publish to all clients" << std::endl;
	
	// avoid exit main thread
	while (true)
		std::this_thread::sleep_for(std::chrono::milliseconds(500));

	return 0;
}