#include "TCP.h"

socket_obj::socket_obj(char* addr, int port)
	:socket_fd_(socket(AF_INET, SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK, IPPROTO_TCP)),
	listened_(false)
{
	bzero(&inet_address_, sizeof inet_address_);
	inet_address_.sin_family = AF_INET;
	inet_address_.sin_addr.s_addr = inet_addr(addr);
	inet_address_.sin_port = htons(static_cast<in_port_t>(port));
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
	inet_address_.sin_port = htons(static_cast<in_port_t>(port));
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

void accpect_socket_obj::set_tcp_no_delay(bool on)
{
	int optva = on ? 1 : 0;
	::setsockopt(socket_fd_,IPPROTO_TCP,TCP_NODELAY,&optva,sizeof optva);
}

int accpect_socket_obj::socket_fd() const
{
	return socket_fd_;
}

connect_socket_obj::connect_socket_obj(char* addr, int port)
	:socket_fd_(socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, IPPROTO_TCP))
{
	bzero(&inet_address_, sizeof inet_address_);
	inet_address_.sin_family = AF_INET;
	inet_address_.sin_addr.s_addr = inet_addr(addr);
	inet_address_.sin_port = htons(static_cast<in_port_t>(port));
}

connect_socket_obj::~connect_socket_obj()
{
	close(socket_fd_);
}

int connect_socket_obj::socket_fd() const
{
	return socket_fd_;
}

int connect_socket_obj::connect()
{
	return ::connect(socket_fd_, (sockaddr*)(&inet_address_), sizeof(inet_address_));
}

int create_nonblocking_socket()
{
	return socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, IPPROTO_TCP);
}
//用来在socket可写时检查连接错误
//如果socket_fd正确，返回opt_val(0) 否则返回 错误代码 或者 opt_val
int check_socket_opt(int socket_fd)
{
	int opt_val;
	socklen_t len=sizeof opt_val;
	//socket_fd有错误so_error是错误代码 否则为0
	if (::getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, &opt_val, &len) < 0)
	{
		return errno;
	}
	else return opt_val;
}
//用来检查自连接
bool is_self_connection(int socket_fd)
{
	sockaddr_in localaddr = get_local_addr(socket_fd);
	sockaddr_in peeraddr = get_peer_addr(socket_fd);
	return (localaddr.sin_addr.s_addr == peeraddr.sin_addr.s_addr&&localaddr.sin_port==peeraddr.sin_port);
}

sockaddr_in get_local_addr(int socket)
{
	sockaddr_in local_addr;
	::bzero(&local_addr, sizeof local_addr);
	socklen_t addr_len = sizeof local_addr;
	if (getsockname(socket, (sockaddr*)(&local_addr), &addr_len)<0)
	{
		LOG_SYSERR << "TCP/get_local_addr";
	}
	return local_addr;
}

sockaddr_in get_peer_addr(int socket)
{
	sockaddr_in peer_addr;
	::bzero(&peer_addr, sizeof peer_addr);
	socklen_t addr_len = sizeof peer_addr;
	if (getpeername(socket, (sockaddr*)(&peer_addr), &addr_len) < 0)
	{
		LOG_SYSERR << "TCP/get_peer_addr";
	}
	return peer_addr;
}
