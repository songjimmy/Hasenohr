#pragma once
//这个类用来接受新的TCP连接，并通过回调通知使用者
//先设计最简单的情况，管理一个连接
#include "TCP.h"
#include "event_loop.h"

#include <functional>
class acceptor
{
public:
	typedef std::function<void(accpect_socket_obj&&, event_loop&)> callback_functor;
	acceptor(socket_obj* socket__,event_loop* loop__,const callback_functor& cb__=NULL);
	acceptor(const acceptor&) = delete;
	acceptor operator=(const acceptor&) = delete;
	void set_callback(const callback_functor& cb__);
	void listen();
	void on_listened();
private:
	socket_obj* socket_;
	event_loop* loop_;
	callback_functor cb_;
	channel listen_channel;
	bool listened;
};
