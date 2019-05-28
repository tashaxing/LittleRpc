#include <iostream>
#include <chrono>
#include "little_rpc.h"

namespace little_rpc
{

// ---- client ---- //
RpcClient::RpcClient():
	m_shoud_close(false),
	m_client_sock(m_ctx, ZMQ_DEALER),
	m_cur_send_id(0),
	m_callback(nullptr)
{
}

RpcClient::~RpcClient()
{
	m_shoud_close = true;
	m_msg_thread.join();
}

void RpcClient::set_callback(RpcClientCallback *callback)
{
	m_callback = callback;
}

void RpcClient::connect(const char *addr)
{
	m_addr = addr;
	
	// build unique identity with time stamp, accurate to 1 ns
	int64_t now = chrono::high_resolution_clock::now().time_since_epoch().count();
	m_conn_id = "cpp_client_" + to_string(now);

	// goto main msg loop
	m_msg_thread = thread(&RpcClient::run, this);
}

void RpcClient::run()
{
	// do connect
	const char *identity = m_conn_id.c_str();
	m_client_sock.setsockopt(ZMQ_IDENTITY, identity, strlen(identity));
	m_client_sock.connect(m_addr.c_str());

	// main loop
	zmq::pollitem_t items[] = {{m_client_sock, 0, ZMQ_POLLIN, 0}};
	try
	{
		while (!m_shoud_close)
		{
			zmq::poll(items, 1, 10); // polling time interval 10ms in every loop
			if (items[0].revents & ZMQ_POLLIN)
				receive();
		}
	}
	catch (exception &e)
	{
		cout << "exception: " << e.what() << endl;
	}
}

void RpcClient::receive()
{
	zmq::message_t msg;
	m_client_sock.recv(&msg);
	string msg_str(static_cast<char *>(msg.data()), msg.size());

	if (m_callback)
		m_callback->on_msg_result(msg_str.c_str());
}

void RpcClient::send(const char *data)
{
	m_client_sock.send(data, strlen(data));
}

// ---- server ---- //
RpcServer::RpcServer():
	m_shoud_close(false),
	m_frontend_sock(m_ctx, ZMQ_ROUTER),
	m_backend_sock(m_ctx, ZMQ_DEALER),
	m_server_sock(m_ctx, ZMQ_DEALER),
	m_callback(nullptr)
{
}

RpcServer::~RpcServer()
{
	m_shoud_close = true;
	m_frontend_sock.close();
	m_msg_thread.join();
}

void RpcServer::set_callback(RpcServerCallback *callback)
{
	m_callback = callback;
}

void RpcServer::start(const char *addr)
{
	// router dealer pipeline and server in the same class
	// consider use several server worker to do load balance in the future

	if (!addr || strlen(addr) == 0)
		return;

	// bind to addr
	try
	{
		m_frontend_sock.bind(addr);
		m_backend_sock.bind("inproc://backend");
	}
	catch (exception &e)
	{
		cout << "rpc bind failed, check if the port is already in use, exception: " << e.what() << endl;
	}

	// start server msg loop
	m_msg_thread = thread(&RpcServer::run, this);

	try
	{
		zmq::proxy(m_frontend_sock, m_backend_sock, nullptr);
	}
	catch (exception &e)
	{
		cout << "proxy exception: " << e.what() << endl;
	}
}

void RpcServer::run()
{
	m_server_sock.connect("inproc://backend");

	try
	{
		while (!m_shoud_close)
			receive();
	}
	catch (exception &e)
	{
		cout << "sever exception: " << e.what() << endl;
	}
}

void RpcServer::receive()
{
	zmq::message_t identity;
	zmq::message_t msg;
	m_server_sock.recv(&identity);
	m_server_sock.recv(&msg);
	string identity_str(static_cast<char *>(identity.data()), identity.size());
	string msg_str(static_cast<char *>(msg.data()), msg.size());

	if (m_callback)
		m_callback->on_rpc_request(identity_str.c_str(), msg_str.c_str());
}

void RpcServer::send(const char *conn_id, const char *data)
{
	// use first frame data as session 
	m_server_sock.send(conn_id, strlen(conn_id), ZMQ_SNDMORE);
	m_server_sock.send(data, strlen(data));
}

void RpcServer::close()
{
	m_shoud_close = true;
	m_frontend_sock.close();
	m_msg_thread.join();
}

// namespace frpc
}