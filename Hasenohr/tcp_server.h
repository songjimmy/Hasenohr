#pragma once
#include "TCP.h"
#include <vector>
#include <map>

#include "tcp_connection.h"
class tcp_server
{
public:
	typedef std::map<std::string,tcp_connection_ptr> tcp_connection_list;
	tcp_server(event_loop* loop__, socket_obj& lisenten_addr_,std::string name);
	void set_connection_callback(connection_callback connection_callback__);
	void set_message_callback(message_callback message_callback__);
	void on_connection(accpect_socket_obj&&,event_loop& loop_);
	void remove_connection(const tcp_connection_ptr& conn);
	void start();
private:
	int conn_id;
	std::string name_;
	event_loop* loop_;
	acceptor acceptor_;
	tcp_connection_list tcp_connection_list_;
	connection_callback connection_callback_;
	message_callback message_callback_;
};