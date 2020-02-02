#include "tcp_connection.h"
using std::placeholders::_1;
using std::placeholders::_2;
tcp_connection::tcp_connection(event_loop* loop__, accpect_socket_obj&& socket_temp, std::string name)
	:name_(name)
{
	state = kConnecting;
	loop_ = loop__;
	socket_fd.reset(new accpect_socket_obj(std::move(socket_temp)));
	conn_channel.reset(new channel(loop__,socket_fd->socket_fd()));
	conn_channel->set_write_callback(std::bind(&tcp_connection::on_writeable, this, std::ref(out_buf)));
	high_watermark_ = 553;
}

tcp_connection::~tcp_connection()
{
}

void tcp_connection::set_connection_callback(const connection_callback& connection_callback__)
{
	connection_callback_ = connection_callback__;
}

void tcp_connection::set_message_callback(const message_callback& message_callback__)
{
	message_callback_ = message_callback__;
	conn_channel->set_read_callback(std::bind(&tcp_connection::on_message,this,_1,std::ref(in_buf)));
}

void tcp_connection::set_write_complete_callback(const connection_callback& write_complete_callback__)
{
	write_complete_callback_ = write_complete_callback__;
}

void tcp_connection::set_disconnect_callback(const connection_callback& disconnect_callback__)
{
	disconnect_callback_ = disconnect_callback__;
	conn_channel->set_close_callback(std::bind(&tcp_connection::on_close, this));
}

void tcp_connection::set_high_watermark_callback(const high_watermark_callback& high_watermark_callback__, ssize_t high_watermark__)
{
	high_watermark_ = high_watermark__;
	high_watermark_callback_ = high_watermark_callback__;
}

void tcp_connection::set_tcp_no_delay(bool on)
{
	this->socket_fd->set_tcp_no_delay(on);
}

void tcp_connection::on_message(muduo::Timestamp timestamp,buffer& buf)
{
	int erro;
	ssize_t n=buf.read_fd(conn_channel->fd(),&erro);
	if (n>0)
	{
		message_callback_(shared_from_this(), timestamp, buf);
	}
	else if(n==0)
	{
		on_close();
	}
	else
	{
	}
}

void tcp_connection::on_writeable(buffer& buf)
{
	int erro;
	buf.write_fd(conn_channel->fd(),high_watermark_,&erro);
	if (buf.readable_size() == 0)
	{
		buf.clear();
		conn_channel->unenable_writing();
		if (write_complete_callback_)
		write_complete_callback_(shared_from_this());
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
	assert((state == kConnected)|| (state == kDisConnecting));//
	conn_channel->unenable_reading();//关闭连接的前置工作，该操作之后
	this->message_callback_ = NULL;
	disconnect_callback_(shared_from_this());
}


void tcp_connection::destory_connection()
{
	loop_->assert_in_loop_thread();
	assert((state == kConnected) || (state == kDisConnecting));
	state = kDisConnected;
	conn_channel->unenable_reading();//关闭连接的前置工作，该操作之后
	connection_callback_(shared_from_this());
	loop_->remove_channel(conn_channel.get());
}

bool tcp_connection::connecting()
{
	return state!=kDisConnected;
}

void tcp_connection::send(const std::string& content)
{
	loop_->run_in_loop(std::bind(&tcp_connection::send_in_loop, this, std::ref(content)));
}

void tcp_connection::shutdown()
{
	if (state == kConnected)
	{
		state=kDisConnecting;
		loop_->run_in_loop(std::bind(&tcp_connection::shut_down_in_loop, this));
	}
}

void tcp_connection::set_state(states s)
{
	state = s;
}

void tcp_connection::send_in_loop(const std::string& content)
{
	if (conn_channel->is_writing())
	{
		out_buf.append(content.c_str(), content.size());
	}
	else
	{
		loop_->assert_in_loop_thread();
		ssize_t send_size = content.size() < high_watermark_ ? content.size() : high_watermark_;
		ssize_t n = ::write(conn_channel->fd(), (void*)(content.c_str()), send_size );
		if (n < 0)
		{
		}
		//全部发送,且send_size小于highwatermark则无需缓存,并触发write_complete_callback
		else if (n <= ssize_t(content.size())&&send_size<ssize_t(high_watermark_))
		{
			if(write_complete_callback_)
			write_complete_callback_(shared_from_this());
		}
		else
		{
			conn_channel->enable_writing();
			LOG_INFO << (content.size() - n) << " bytes will be cache in outbuf";
			out_buf.append(content.c_str()+n, content.size() - n);
			if (send_size < ssize_t(content.size()) && high_watermark_callback_)
			{
				high_watermark_callback_(shared_from_this(), content.size() - n);
			}
		}
	}
}

void tcp_connection::shut_down_in_loop()
{
	loop_->assert_in_loop_thread();
	if (!conn_channel->is_writing())
	{
		//关闭写
		::shutdown(conn_channel->fd(), SHUT_WR);
	}
}

