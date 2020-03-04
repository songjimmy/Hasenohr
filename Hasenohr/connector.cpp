#include "connector.h"
int connector::init_retry_delay_ms = 500;
int connector::max_retry_delay_ms = 30000;
connector::connector(event_loop* loop__,const char* addr, int port)
	:loop_(loop__),service_addr(addr,port),state_(k_disconnected),
	connect_(false),retry_delay_ms(connector::init_retry_delay_ms)
	
{
	
}

void connector::start()
{
	connect_ = true;
	loop_->queue_in_loop(std::bind(&connector::start_in_loop,this));
}

void connector::restart()
{
	//参数的重置，然后重新启动
	loop_->assert_in_loop_thread();
	state_ = k_disconnected;
	connect_ = true;
	retry_delay_ms = init_retry_delay_ms;
	start_in_loop();
}

void connector::stop()
{
	connect_ = false;
	//这里负责相关计时器的析构
	timer_queue::timer_ptr time_ptr_ = timer_.lock();
	if (time_ptr_)
	{
		time_ptr_->set_invalid();
	}
}

void connector::set_new_connection_callback(const new_connection_callback& cb)
{
	new_connection_callback_ = cb;
}

void connector::start_in_loop()
{
	loop_->assert_in_loop_thread();
	assert(state_ == k_disconnected);
	if (connect_)
	{
		connect();
	}
	else
	{
		LOG_SYSERR << "do not connect";
	}
}

void connector::connect()
{
	int socket_fd = create_nonblocking_socket();
	int ret=service_addr.connect_at_this_address(socket_fd);
	int saved_errno = (ret == 0) ? 0 : errno;
	//这里的核心问题 errno 宏是干嘛的？错误代码 系统最后一次错误的相关信息
	switch (saved_errno)
	{
		//connect的错误代码 这里是正常的处理方式 进行channel的建立操作
	case 0:
	case EINPROGRESS:
	case EINTR:
	case EISCONN:
		connecting(socket_fd);
		break;
		//这里出现的都是可以通过重启连接解决的问题,可以通过等待解决的问题
	case EAGAIN:
	case EADDRINUSE:
	case EADDRNOTAVAIL:
	case ECONNREFUSED:
	case ENETUNREACH:

		LOG_DEBUG << "error code:"<<saved_errno;
		retry(socket_fd);//socket_fd的关闭和新连接的尝试建立
		break;
		//不可恢复的错误，被拒绝 ，参数错误
	case EACCES:
	case EPERM:
	case EAFNOSUPPORT:
	case EALREADY:
	case EBADF:
	case EFAULT:
	case ENOTSOCK:
		LOG_SYSERR << "connect error in connector::start_in_loop ---> connect" << saved_errno;
		::close(socket_fd);
		LOG_SYSERR << " " << socket_fd << " is closed.";
		break;
	default:
		LOG_SYSERR << "unexpected connect error in connector::start_in_loop ---> connect" << saved_errno;
		::close(socket_fd);
		LOG_SYSERR << " " << socket_fd << " is closed.";
		break;
	}
}

void connector::connecting(int socketfd)
{
	state_ = k_connecting;
	//检查是否会造成channel的意外析构
	assert(!channel_);
	channel_.reset(new channel(loop_, socketfd));
	channel_->set_write_callback(std::bind(&connector::handle_write,this));
	channel_->set_erro_callback(std::bind(&connector::handle_error,this));
	channel_->enable_writing();
}
//该函数总是绑定到channel_的write_able_callback中进行使用，其在socket_fd具有可写性的情况下
//检查是否是成功的连接，该函数有两种分支，一种要关闭socket
//一种要为客户端新建一个连接tcp_connection,确保这两种情况下都不存在文件操作符的泄露
void connector::handle_write()
{
	//这是绑定到channel中执行的函数，从这个角度考虑其安全性
	//连接channel中的socket变为可写，之后这个channel就没有用了//如果成功会新建一个连接，连接中包含这个套接字
	//如果失败 直接用retry关闭？
	if (state_ == k_connecting)
	{
		int socket_fd = remove_and_reset_channel();
		int err = check_socket_opt(socket_fd);
		if (err)
		{
			//常规的错误处理
			LOG_INFO << "retry for error: "<<err;
			retry(socket_fd);
		}
		else if (is_self_connection(socket_fd))
		{
			LOG_INFO << "retry for self connection";
			retry(socket_fd);
		}
		else
		{
			if (connect_)
			{
				//传入socket_fd新建连接
				new_connection_callback_(socket_fd);
			}
			else
			{
				//无事发生
			}
		}
	}
	else
	{
		assert(state_ == k_disconnected);
	}
}
void connector::handle_error()
{
	if (k_connecting == state_)
	{
		int socket_fd = remove_and_reset_channel();
		LOG_INFO << "connector::handle_error : can't connect to  " << service_addr.show_addr();
		retry(socket_fd);
	}
}
//负责socketfd文件标识符的关闭
void connector::retry(int socketfd)
{
	::close(socketfd);
	state_ = k_disconnected;
	if (connect_)
	{
		LOG_INFO << "connector::retry-Retry connecting to: "
			<< service_addr.show_addr() << ":" << service_addr.show_port()
			<< " after " << retry_delay_ms << " MSs.";
			//timer_Id 考虑channel析构后的问题
		timer_=loop_->run_after(std::bind(&connector::connect,this),retry_delay_ms/1000);
		retry_delay_ms = std::min(retry_delay_ms*2,connector::max_retry_delay_ms);
	}
	else
	{
		LOG_DEBUG << "do not connect";
	}
}
//始终在hanle_write中调用 即 始终被channel的成员函数调用 可以使用loop_->remove_channel，因为是对指针的操作，不可以使用
int connector::remove_and_reset_channel()
{
	channel_->unenable_all();
	loop_->assert_in_loop_thread();
	assert(state_ == k_connecting);
	loop_->remove_channel(channel_.get());
	int socket_fd=channel_->fd();
	loop_->queue_in_loop(std::bind(&connector::reset_channel, this));
	return socket_fd;
}
//只能被remove_and_reset_channel绑定到loop的额外事件中进行执行
void connector::reset_channel()
{
	channel_.reset();
}


