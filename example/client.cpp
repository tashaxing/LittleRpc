#include <iostream>
#include "little_rpc.h"

using namespace little_rpc;

class MyClientCallback : public RpcClientCallback
{
public:
	MyClientCallback()
	{
		// contain a rpc client
		m_client = new RpcClient;
		m_client->set_callback(this);
	}

	~MyClientCallback()
	{
		delete m_client;
		m_client = NULL;
	}

	void bind_client(RpcClient* client)
	{
		// save client and set self as client callback
		m_client = client;
		m_client->set_callback(this);
	}

	virtual void on_connected()
	{
		// TODO: for heartbeat
	}

	virtual void on_disconnected()
	{
		// TODO: for heartbeat
	}

	virtual void on_msg_result(const char *msg_data)
	{
		// here you can dispatch msg with self defined call_id
		std::cout << "on_msg_result: " << msg_data << std::endl;
	}

	void connect_server(const char* addr)
	{
		// FIXME: need sync check connect status
		m_client->connect(addr);
	}

	void send_request(const char* data)
	{
		m_client->send(data);
	}

private:
	RpcClient* m_client;
};

int main()
{
	// specify sever address
	const char* addr = "tcp://127.0.0.1:5000";

	MyClientCallback my_client; // the real client handler

	// connect
	my_client.connect_server(addr);
	std::cout << "connect to " << addr << std::endl;

	// test send data
	my_client.send_request("request for lucky day");
	std::cout << "send to server" << std::endl;

	// avoid exit main thread if you want to receive notification
	while (true)
		std::this_thread::sleep_for(std::chrono::milliseconds(500));

	return 0;
}