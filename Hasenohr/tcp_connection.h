#pragma once
#include <memory>
#include <string>

#include "muduo/base/Logging.h"

#include "acceptor.h"

//һ��tcp_connection����һ�����ӣ��ѽ����׽��֣����������ڵ���������ӣ�
class tcp_connection;
typedef std::shared_ptr<tcp_connection> tcp_connection_ptr;
typedef std::function<void(tcp_connection_ptr)> connection_callback;
typedef std::function<void(tcp_connection_ptr, std::string)> message_callback;
class tcp_connection :public std::enable_shared_from_this<tcp_connection>//����ļ̳���Ϊ���ڻص���ע��shared_ptr
{
public:
	typedef std::shared_ptr<tcp_connection> tcp_connection_ptr;
	tcp_connection(event_loop* loop__,accpect_socket_obj&& socket_temp,std::string name);
	~tcp_connection();
	void set_connection_callback(const connection_callback& connection_callback__);
	void set_message_callback(const message_callback& message_callback__);
	void set_disconnect_callback(const connection_callback& disconnection_callback__);
	void on_message(tcp_connection_ptr conn, std::string);
	void connection_established();
	void on_close();
	void destory_connection();
	const std::string name_;
private:
	//���״̬���Է�ֹconnection_established()���õڶ���
	enum states
	{
		kConnecting,
		kConnected,
		kDisConnected
	};
	void set_state(states s);
	states state;
	std::string buf;
	std::unique_ptr<accpect_socket_obj> socket_fd;
	event_loop* loop_;
	std::unique_ptr<channel> conn_channel;
	//���������ַ�ͶԵ������ַ
	//socket_obj local_addr_;
	//socket_obj peer_addr_;
	connection_callback connection_callback_;
	message_callback message_callback_;
	connection_callback disconnect_callback_;
};