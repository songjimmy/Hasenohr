#include "tcp_server.h"
using std::placeholders::_1;
using std::placeholders::_2;

tcp_server::tcp_server(event_loop* loop__, socket_obj& lisenten_addr_ ,std::string name)
	:conn_id(0),name_(name),loop_(loop__), acceptor_(&lisenten_addr_, loop__)
{
	acceptor::callback_functor cb(std::bind(&tcp_server::on_connection,this,_1,_2));
	acceptor_.set_callback(cb);
}

void tcp_server::set_connection_callback(connection_callback connection_callback__)
{
	connection_callback_ = connection_callback__;
}

void tcp_server::set_message_callback(message_callback message_callback__)
{
	message_callback_ = message_callback__;
}

void tcp_server::on_connection(accpect_socket_obj&& accpect_socket_obj__, event_loop& loop_)
{
	std::string conn_name = name_ + std::to_string(conn_id);
	++conn_id;
	LOG_INFO << "connection "<<conn_name<<"is estiblish";
	auto ret_pair=tcp_connection_list_.insert({conn_name, std::shared_ptr<tcp_connection>(new tcp_connection(&loop_, std::move(accpect_socket_obj__), conn_name)) });
	assert(ret_pair.second);
	auto new_conn = ret_pair.first->second;
	new_conn->set_connection_callback(connection_callback_);
	new_conn->set_message_callback(message_callback_);
	new_conn->set_disconnect_callback(std::bind(&tcp_server::remove_connection,this,_1));
	new_conn->connection_established();
}
//
//从连接列表中删除tcp_conn_Ptr
void tcp_server::remove_connection(const tcp_connection_ptr& conn)
{
	ssize_t n = tcp_connection_list_.erase(conn->name_);
	assert(n == 1);
	loop_->queue_in_loop(std::bind(&tcp_connection::destory_connection,conn));//conn的析构
}

void tcp_server::start()
{
	acceptor_.listen();
}
