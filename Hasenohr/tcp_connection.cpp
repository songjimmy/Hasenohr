#include "tcp_connection.h"

tcp_connection::tcp_connection(event_loop* loop__, accpect_socket_obj&& socket_temp, std::string name)
	:name_(name)
{
	state = kConnecting;
	loop_ = loop__;
	socket_fd.reset(new accpect_socket_obj(std::move(socket_temp)));
	conn_channel.reset(new channel(loop__,socket_fd->socket_fd()));
}

tcp_connection::~tcp_connection()
{
	LOG_INFO<<"~tcp_connection";
}

void tcp_connection::set_connection_callback(const connection_callback& connection_callback__)
{
	connection_callback_ = connection_callback__;
}

void tcp_connection::set_message_callback(const message_callback& message_callback__)
{
	message_callback_ = message_callback__;
	conn_channel->set_read_callback(std::bind(&tcp_connection::on_message,this, shared_from_this(),buf));
}

void tcp_connection::set_disconnect_callback(const connection_callback& disconnect_callback__)
{
	disconnect_callback_ = disconnect_callback__;
}

void tcp_connection::on_message(tcp_connection_ptr conn, std::string buf)
{
	char buff[100];
	bzero(buff, sizeof buff);
	ssize_t n=recv(conn->socket_fd->socket_fd(), buff, 100, 0);
	if (n>0)
	{
		buf = buff;
		message_callback_(conn, buf);
	}
	else if(n==0)
	{
		disconnect_callback_(conn);
	}
}

void tcp_connection::connection_established()
{
	loop_->assert_in_loop_thread();
	assert(state == kConnecting);
	state = kConnected;
	connection_callback_(shared_from_this());
	conn_channel->enable_reading();
}
//用来处理关闭，调用disconnect_callback
void tcp_connection::on_close()
{
	loop_->assert_in_loop_thread();
	assert(state == kConnected);//
	conn_channel->unenable_reading();//关闭连接的前置工作，该操作之后
	disconnect_callback_(shared_from_this());
}

void tcp_connection::destory_connection()
{
	loop_->assert_in_loop_thread();
	assert(state == kConnected);
	state = kDisConnected;
	conn_channel->unenable_reading();//关闭连接的前置工作，该操作之后
	connection_callback_(shared_from_this());
	loop_->remove_channel(conn_channel.get());
}

void tcp_connection::set_state(states s)
{
	state = s;
}

