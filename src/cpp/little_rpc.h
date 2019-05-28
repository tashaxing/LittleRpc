#ifndef _LITTLE_RPC_H
#define _LITTLE_RPC_H

#include <thread>
#include <memory>
#include <string> 
#include <string.h>
#include "zmq.hpp"

namespace little_rpc
{
using namespace std;

// support JSON and PROTOBUF, parsed in application level
enum MsgType
{
	JSON,
	PROTOBUF
};

class RpcClientCallback
{
public:
	virtual void on_connected() = 0;
	virtual void on_disconnected() = 0;
	virtual void on_msg_result(const char *msg_data) = 0;
};

class RpcClient
{
public:
	RpcClient();
	virtual ~RpcClient();

	void set_callback(RpcClientCallback *callback);
	void connect(const char *addr);
	void send(const char *data);
	void receive();

private:
	void run();

private:
	string m_addr;
	zmq::context_t m_ctx;
	zmq::socket_t m_client_sock;
	thread m_msg_thread;
	string m_conn_id;
	int m_cur_send_id;
	volatile bool m_shoud_close;

	RpcClientCallback *m_callback;
};

class RpcServerCallback
{
public:
	virtual void on_rpc_request(const char *conn_id, const char *req_data) = 0;
	virtual void on_close(const char *conn_id) = 0;
};

class RpcServer
{
public:
	RpcServer();
	virtual ~RpcServer();

	void set_callback(RpcServerCallback *callback);
	void start(const char *addr);
	void send(const char *conn_id, const char *data);
	void close();

protected:
	void run();
	void receive();

private:
	zmq::context_t m_ctx;
	zmq::socket_t m_frontend_sock;
	zmq::socket_t m_backend_sock;

	thread m_msg_thread;
	zmq::socket_t m_server_sock;
	volatile bool m_shoud_close;

	RpcServerCallback *m_callback;
};


} // namespace little_rpc

#endif // !_LITTLE_RPC_H