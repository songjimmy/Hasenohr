#pragma once
#include "TCP.h"
#include <vector>
#include <map>

#include "event_loop_thread.h"
#include "tcp_connection.h"
class tcp_server
{
public:
	typedef std::map<std::string,tcp_connection_ptr> tcp_connection_list;
	tcp_server(event_loop* loop__, socket_obj& lisenten_addr_,std::string name);
	void set_connection_callback(connection_callback connection_callback__);
	void set_message_callback(message_callback message_callback__);
	void set_write_complete_callback(connection_callback write_complete_callback__);
	void set_high_watermark_callback(high_watermark_callback high_watermark_callback__,ssize_t high_watermark__);
	void on_connection(accpect_socket_obj&&);//,event_loop& loop_);
	void remove_connection(const tcp_connection_ptr& conn);
	void start();
private:
	void remove_connection_in_loop(const tcp_connection_ptr& conn);
	int conn_id;
	std::string name_;
	event_loop* loop_;
	//在事件循环线程池实现之前，服务器的主线程负责连接的建立，一个子线程负责io事件的实现
	event_loop_thread io_loop_thread_;
	event_loop* io_loop_;
	acceptor acceptor_;
	ssize_t high_watermark_;
	tcp_connection_list tcp_connection_list_;
	connection_callback connection_callback_;
	connection_callback write_complete_callback_;
	high_watermark_callback high_watermark_callback_;
	message_callback message_callback_;
};