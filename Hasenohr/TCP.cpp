#include "TCP.h"

socket_obj::socket_obj(char* addr, int port)
	:socket_fd_(socket(AF_INET, SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK, IPPROTO_TCP)),
	listened_(false)
{
	bzero(&inet_address_, sizeof inet_address_);
	inet_address_.sin_family = AF_INET;
	inet_address_.sin_addr.s_addr = inet_addr(addr);
	inet_address_.sin_port = htons(port);
	int bind_flag=bind(socket_fd_, (sockaddr*)(&inet_address_), sizeof(inet_address_));
	assert(bind_flag >= 0);
}

socket_obj::socket_obj(int port)
	:socket_fd_(socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, IPPROTO_TCP)),
	listened_(false)
{
	bzero(&inet_address_, sizeof inet_address_);
	inet_address_.sin_family = AF_INET;
	inet_address_.sin_addr.s_addr = INADDR_ANY;
	inet_address_.sin_port = htons(port);
	int bind_flag = bind(socket_fd_, (sockaddr*)(&inet_address_), sizeof(inet_address_));
	assert(bind_flag >= 0);
}

void socket_obj::listen(int queue_num)
{
	assert(!listened_);
	listened_ = true;
	int listen_flag=::listen(socket_fd_, queue_num);
	assert(listen_flag >=0);
}

int socket_obj::socket_fd() const
{
	return socket_fd_;
}

bool socket_obj::listened()
{
	return listened_;
}

accpect_socket_obj socket_obj::accpect()
{
	assert(listened_);
	socklen_t socket_size = sizeof(inet_address_);
	return std::move(accpect_socket_obj(::accept(socket_fd_, (sockaddr*)(&inet_address_), &(socket_size))));
}


socket_obj::~socket_obj()
{
	::close(socket_fd_);
}

accpect_socket_obj::accpect_socket_obj(int fd)
	:socket_fd_(fd)
{}

accpect_socket_obj::accpect_socket_obj(accpect_socket_obj&& obj)
:socket_fd_(obj.socket_fd_)
{
	obj.socket_fd_ = -1;
}

accpect_socket_obj::~accpect_socket_obj()
{
	if (socket_fd_ >= 0)
	::close(socket_fd_);
}

int accpect_socket_obj::socket_fd() const
{
	return socket_fd_;
}
