#pragma once
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <assert.h>
#include <algorithm>

#define DEFAULT_QUEUE_NUM (int(1024)) 
class accpect_socket_obj;
class socket_obj
{
public:
	socket_obj(char* addr,int port);
	socket_obj(int port);
	socket_obj(const socket_obj&) = delete;
	socket_obj operator=(const socket_obj&) = delete;
	void listen(int queue_num = DEFAULT_QUEUE_NUM);
	int socket_fd() const;
	bool listened();
	accpect_socket_obj accpect();
	~socket_obj();
private:
	//套接字地址结构体
	int socket_fd_;
	bool listened_;
	sockaddr_in inet_address_;
};
class accpect_socket_obj
{
public:
	accpect_socket_obj(int fd);
	accpect_socket_obj(const accpect_socket_obj&) = delete;
	accpect_socket_obj(accpect_socket_obj&&);
	accpect_socket_obj operator=(const accpect_socket_obj&)=delete;
	~accpect_socket_obj();
	int socket_fd() const;
private:
	int socket_fd_;
};