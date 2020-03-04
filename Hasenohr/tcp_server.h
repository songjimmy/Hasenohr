#pragma once
#include "TCP.h"
#include <vector>
#include <map>
#include "event_loop_thread.h"
#include "tcp_connection.h"
#include "event_loop_thread_pool.h"
#define THREAD_SIZE 4; 
class tcp_server
{
public:
	typedef std::map<std::string,tcp_connection_ptr> tcp_connection_list;
	tcp_server(event_loop* loop__, socket_obj& lisenten_addr_,std::string name, size_t threads_size_= 4);
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
	event_loop_thread_pool threads_pool;
	acceptor acceptor_;
	ssize_t high_watermark_;
	tcp_connection_list tcp_connection_list_;
	connection_callback connection_callback_;
	connection_callback write_complete_callback_;
	high_watermark_callback high_watermark_callback_;
	message_callback message_callback_;
};