#pragma once
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <cstring>
#include <assert.h>
#include <algorithm>
#include <string>
class inet_address
{
public:
	inet_address(const char* addr_, int port_)
		:addr(addr_),port(port_)
	{
		bzero((void*)(&inet_address_), sizeof inet_address_);
		inet_address_.sin_family = AF_INET;
		inet_address_.sin_addr.s_addr = inet_addr(addr);
		inet_address_.sin_port = htons(static_cast<in_port_t>(port));
	}
	int connect_at_this_address(int socket_fd) const
	{
		return connect(socket_fd, (sockaddr*)&inet_address_, sizeof(inet_address_));
	}
	std::string show_addr() { return std::string(addr); }
	int show_port() { return port; }
private:
	const char* addr;
	int port;
	sockaddr_in inet_address_;
};
