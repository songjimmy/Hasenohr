#pragma once
#include <memory>
#include <string>

#include <muduo/base/Logging.h>
#include <muduo/base/Timestamp.h>

#include "acceptor.h"
#include "buffer.h"

//一个tcp_connection管理一个连接（已接受套接字），生命周期等于这个连接，
class tcp_connection;
typedef std::shared_ptr<tcp_connection> tcp_connection_ptr;
typedef std::function<void(const tcp_connection_ptr&)> connection_callback;
typedef std::function<void(const tcp_connection_ptr&,ssize_t)> high_watermark_callback;
typedef std::function<void(const tcp_connection_ptr&,muduo::Timestamp, buffer&)> message_callback;
class tcp_connection :public std::enable_shared_from_this<tcp_connection>//这里的继承是为了在回调中注册shared_ptr
{
public:
	typedef std::shared_ptr<tcp_connection> tcp_connection_ptr;
	tcp_connection(event_loop* loop__,accpect_socket_obj&& socket_temp,std::string name);
	~tcp_connection();
	void set_connection_callback(const connection_callback& connection_callback__);
	void set_message_callback(const message_callback& message_callback__);
	void set_write_complete_callback(const connection_callback& write_complete_callback__);
	void set_disconnect_callback(const connection_callback& disconnection_callback__);
	void set_high_watermark_callback(const high_watermark_callback& high_watermark_callback__, ssize_t high_watermark__);
	void set_tcp_no_delay(bool on);
	void on_message(muduo::Timestamp timestamp, buffer& buf);
	void on_writeable(buffer& buf);
	void connection_established();
	void on_close();
	void destory_connection();
	bool connecting();
	void send(const std::string& content);
	void shutdown();
	const std::string name_;
private:
	//这个状态可以防止connection_established()调用第二次
	enum states
	{
		kConnecting,
		kConnected,
		kDisConnecting,
		kDisConnected
	};
	void set_state(states s);
	void send_in_loop(const std::string& content);
	void shut_down_in_loop();
	states state;
	buffer in_buf;
	buffer out_buf;
	std::unique_ptr<accpect_socket_obj> socket_fd;
	event_loop* loop_;
	std::unique_ptr<channel> conn_channel;
	//本地网络地址和对点网络地址
	//socket_obj local_addr_;
	//socket_obj peer_addr_;
	size_t high_watermark_;
	connection_callback connection_callback_;
	connection_callback write_complete_callback_;
	high_watermark_callback high_watermark_callback_;
	message_callback message_callback_;
	connection_callback disconnect_callback_;
};