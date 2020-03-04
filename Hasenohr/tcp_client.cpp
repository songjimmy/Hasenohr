#include "tcp_client.h"

tcp_client::tcp_client(event_loop* loop__, std::string addr_, int port, std::string name__)
	:name_(name__),loop_(loop__),connector_(loop_,addr_.c_str(),port)
{
	high_watermark_ = 66536;
}

void tcp_client::set_connection_callback(connection_callback connection_callback__)
{
	connection_callback_ = connection_callback__;
}

void tcp_client::set_message_callback(message_callback message_callback__)
{
	message_callback_ = message_callback__;
}

void tcp_client::set_write_complete_callback(connection_callback write_complete_callback__)
{
	write_complete_callback_ = write_complete_callback__;
}

void tcp_client::set_high_watermark_callback(high_watermark_callback high_watermark_callback__, ssize_t high_watermark__)
{
	high_watermark_callback_ = high_watermark_callback__;
	high_watermark_ = high_watermark__;
}

void tcp_client::on_connection(accpect_socket_obj&& socket_)
{	
	//assert(conn_.get() == nullptr);
	conn_.reset(new tcp_connection(loop_, std::move(socket_), name_));
	conn_->set_connection_callback(connection_callback_);
	conn_->set_message_callback(message_callback_);
	conn_->set_disconnect_callback(std::bind(&tcp_client::shut_down, this));
	conn_->set_write_complete_callback(write_complete_callback_);
	conn_->set_high_watermark_callback(high_watermark_callback_, high_watermark_);
	loop_->run_in_loop(std::bind(&tcp_connection::connection_established,conn_));
}

void tcp_client::start()
{
	connector_.set_new_connection_callback(std::bind(&tcp_client::on_connection,this,_1));
	connector_.start();
}

void tcp_client::shut_down()
{
	loop_->run_in_loop(std::bind(&tcp_client::shut_down_in_loop,this));
}

void tcp_client::shut_down_in_loop()
{
	loop_->queue_in_loop(std::bind(&tcp_connection::destory_connection, conn_));
	conn_.reset();
}
