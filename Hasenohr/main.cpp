#include "event_loop.h"
#include "channel.h"
#include "poller.h"
#include "timer_queue.h"
#include "event_loop_thread.h"
#include "acceptor.h"
#include "tcp_server.h"

#include <muduo/base/Thread.h>
#include <muduo/base/Logging.h>

#include <sys/timerfd.h>
#include <stdio.h>

#include <signal.h>

#include <functional>
using std::placeholders::_1;
using std::placeholders::_2;
std::vector<std::string> vec_content;
void on_connection(const tcp_connection_ptr& conn)
{
	if (conn->connecting())
	{
		printf("connection %s is on\n", conn->name_.c_str());
		conn->send(*vec_content.begin());
	}
	else
		printf("connection %s is down\n", conn->name_.c_str());
}
void on_message_callback(const tcp_connection_ptr& conn, muduo::Timestamp timestamp, buffer& buf)
{
	double time_diff = muduo::timeDifference(muduo::Timestamp::now(), timestamp);
	printf("client said:\n");
	std::string content = buf.recv_as_string();
	printf(content.c_str());
	buf.clear();
	printf("\nbefore %f seconds\n",time_diff);
	conn->send(content);
	conn->set_tcp_no_delay(1);
}
void on_write_complete(const tcp_connection_ptr& conn)
{
	printf("write complete, no date in outbuff\n");
	if (vec_content.empty())
	{
		printf("send finish\n");
		conn->shutdown();
	}
	else
	{
		vec_content.erase(vec_content.begin());
		conn->send(*vec_content.begin());
	}
}
void on_high_watermark(const tcp_connection_ptr& conn, ssize_t len)
{
	printf("%d\n", len);
}
int main()
{
	char i = 'a';
	while (i <= 'z')
	{
		vec_content.push_back(std::string(5000, i));
		vec_content.back() += "end\n";
		++i;
	}
	event_loop loop_;
	socket_obj listen_addr(8888);
	tcp_server server(&loop_,listen_addr,"server");
	server.set_connection_callback(on_connection);
	//server.set_message_callback(on_message_callback);
	server.set_write_complete_callback(on_write_complete);
	server.set_high_watermark_callback(on_high_watermark, 1000);
	server.start();
	loop_.loop();
}