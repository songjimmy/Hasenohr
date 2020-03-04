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
	//���������ã�Ȼ����������
	loop_->assert_in_loop_thread();
	state_ = k_disconnected;
	connect_ = true;
	retry_delay_ms = init_retry_delay_ms;
	start_in_loop();
}

void connector::stop()
{
	connect_ = false;
	//���︺����ؼ�ʱ��������
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
	//����ĺ������� errno ���Ǹ���ģ�������� ϵͳ���һ�δ���������Ϣ
	switch (saved_errno)
	{
		//connect�Ĵ������ �����������Ĵ���ʽ ����channel�Ľ�������
	case 0:
	case EINPROGRESS:
	case EINTR:
	case EISCONN:
		connecting(socket_fd);
		break;
		//������ֵĶ��ǿ���ͨ���������ӽ��������,����ͨ���ȴ����������
	case EAGAIN:
	case EADDRINUSE:
	case EADDRNOTAVAIL:
	case ECONNREFUSED:
	case ENETUNREACH:

		LOG_DEBUG << "error code:"<<saved_errno;
		retry(socket_fd);//socket_fd�Ĺرպ������ӵĳ��Խ���
		break;
		//���ɻָ��Ĵ��󣬱��ܾ� ����������
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
	//����Ƿ�����channel����������
	assert(!channel_);
	channel_.reset(new channel(loop_, socketfd));
	channel_->set_write_callback(std::bind(&connector::handle_write,this));
	channel_->set_erro_callback(std::bind(&connector::handle_error,this));
	channel_->enable_writing();
}
//�ú������ǰ󶨵�channel_��write_able_callback�н���ʹ�ã�����socket_fd���п�д�Ե������
//����Ƿ��ǳɹ������ӣ��ú��������ַ�֧��һ��Ҫ�ر�socket
//һ��ҪΪ�ͻ����½�һ������tcp_connection,ȷ������������¶��������ļ���������й¶
void connector::handle_write()
{
	//���ǰ󶨵�channel��ִ�еĺ�����������Ƕȿ����䰲ȫ��
	//����channel�е�socket��Ϊ��д��֮�����channel��û������//����ɹ����½�һ�����ӣ������а�������׽���
	//���ʧ�� ֱ����retry�رգ�
	if (state_ == k_connecting)
	{
		int socket_fd = remove_and_reset_channel();
		int err = check_socket_opt(socket_fd);
		if (err)
		{
			//����Ĵ�����
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
				//����socket_fd�½�����
				new_connection_callback_(socket_fd);
			}
			else
			{
				//���·���
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
//����socketfd�ļ���ʶ���Ĺر�
void connector::retry(int socketfd)
{
	::close(socketfd);
	state_ = k_disconnected;
	if (connect_)
	{
		LOG_INFO << "connector::retry-Retry connecting to: "
			<< service_addr.show_addr() << ":" << service_addr.show_port()
			<< " after " << retry_delay_ms << " MSs.";
			//timer_Id ����channel�����������
		timer_=loop_->run_after(std::bind(&connector::connect,this),retry_delay_ms/1000);
		retry_delay_ms = std::min(retry_delay_ms*2,connector::max_retry_delay_ms);
	}
	else
	{
		LOG_DEBUG << "do not connect";
	}
}
//ʼ����hanle_write�е��� �� ʼ�ձ�channel�ĳ�Ա�������� ����ʹ��loop_->remove_channel����Ϊ�Ƕ�ָ��Ĳ�����������ʹ��
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
//ֻ�ܱ�remove_and_reset_channel�󶨵�loop�Ķ����¼��н���ִ��
void connector::reset_channel()
{
	channel_.reset();
}


