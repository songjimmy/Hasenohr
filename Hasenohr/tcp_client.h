#pragma once
#include <vector>

#include "event_loop.h"
#include "connector.h"
#include "tcp_connection.h"
using std::placeholders::_1;
class tcp_client
{
public:
	tcp_client(event_loop* loop__, std::string addr_, int port ,std::string name__ );
	tcp_client(const tcp_client&) = delete;
	void operator=(const tcp_client&) = delete;
	//~tcp_client();
	// 回调函数注册的接口
	void set_connection_callback(connection_callback connection_callback__);
	void set_message_callback(message_callback message_callback__);
	void set_write_complete_callback(connection_callback write_complete_callback__);
	void set_high_watermark_callback(high_watermark_callback high_watermark_callback__, ssize_t high_watermark__);
	void on_connection(accpect_socket_obj&&);
	void start();
	void shut_down();
private:

	void shut_down_in_loop();
	std::string name_;
	event_loop* loop_;
	connector connector_;
	ssize_t high_watermark_;
	tcp_connection_ptr conn_;
	//
	connection_callback connection_callback_;
	connection_callback write_complete_callback_;
	high_watermark_callback high_watermark_callback_;
	message_callback message_callback_;
};