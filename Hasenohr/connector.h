#pragma once
#include "inet_address.h"
#include "channel.h"
#include "TCP.h"
#include "event_loop.h"
#include "muduo/base/Logging.h"
#include "muduo/base/LogStream.h"
#include <memory>
#include <functional>
#include <algorithm>
class connector
{
public:
	typedef std::function<void(int)> new_connection_callback;
	enum state
	{
		k_disconnected, k_connecting, k_connected
	};
	connector(event_loop* loop__,const char* addr,int port);
	connector(const connector&) = delete;
	//析构函数应当怎么写？
	void operator=(const connector&) = delete;
	void start();//线程安全 因为包装了非安全函数将其置于loop线程内使用
	void restart();//非安全，必须在loop线程内使用//考虑是否需要给用户这一接口
	void stop();//非安全
	void set_new_connection_callback(const new_connection_callback& cb);
private:
	static int init_retry_delay_ms;
	static int max_retry_delay_ms;
	void start_in_loop();
	void connect();//通过调用inet_address::connect() 调用linux的::connect()并进行错误处理 连接瞬间的检查 此时channel还没有建立 该函数是处理不经过等待的信息
	void connecting(int socketfd);//在connect()中使用 使用connect()中生成的socketfd 为新的连接尝试创建新的channel(socket_fd),设置应对写操作和错误位的动作
	//linux 主动套接字socket_fd 在非阻塞模式下 连接建立或者失败都是可写的 连接建立也要考虑检查自连接情况 面试可能问到的问题
	void handle_write();//如上所述 判断失败 自连接 成功然后进行相应操作
	void handle_error();//打印错误并重试 重试方法将负责socket的关闭
	void retry(int socketfd);//失败 错误 自连接socket的关闭 如果需要 计时器到期时重新启动连接
	int remove_and_reset_channel();//channel_智能指针的重置 ，且析构指向的channel 为什么不能在语句块内析构channel？
	//考虑reset channel_之后的不安全情况  ：：语句块时channel中绑定的回调函数的一部分 不能在其执行过程中析构自身
	void reset_channel();//channel的析构 queue_in_loop
	
	event_loop* loop_;
	inet_address service_addr;
	std::unique_ptr<channel> channel_;
	state state_;
	bool connect_;//描述是否在联络该地址
	new_connection_callback new_connection_callback_;
	int retry_delay_ms;//
	//计时器组件
	timer_queue::timer_weak timer_;
	
};