#include "acceptor.h"
using std::placeholders::_1;
acceptor::acceptor(socket_obj* socket__, event_loop* loop__,const callback_functor& cb__)
	:
	listened(false),
	cb_(cb__),
	listen_channel(loop__,socket__->socket_fd())
{
	loop_ = loop__;
	socket_ = socket__;
	listen_channel.set_read_callback(std::bind(&acceptor::on_listened, this));
}

void acceptor::set_callback(const callback_functor& cb__)
{
	cb_ = cb__;
}

void acceptor::listen()
{
	loop_->assert_in_loop_thread();
	listened = true;
	socket_->listen();
	listen_channel.enable_reading();
}

void acceptor::on_listened()
{
	accpect_socket_obj accpect_fd=socket_->accpect();
	cb_(std::move(accpect_fd),*loop_);
}
