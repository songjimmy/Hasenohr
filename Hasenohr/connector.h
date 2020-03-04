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
	//��������Ӧ����ôд��
	void operator=(const connector&) = delete;
	void start();//�̰߳�ȫ ��Ϊ��װ�˷ǰ�ȫ������������loop�߳���ʹ��
	void restart();//�ǰ�ȫ��������loop�߳���ʹ��//�����Ƿ���Ҫ���û���һ�ӿ�
	void stop();//�ǰ�ȫ
	void set_new_connection_callback(const new_connection_callback& cb);
private:
	static int init_retry_delay_ms;
	static int max_retry_delay_ms;
	void start_in_loop();
	void connect();//ͨ������inet_address::connect() ����linux��::connect()�����д����� ����˲��ļ�� ��ʱchannel��û�н��� �ú����Ǵ��������ȴ�����Ϣ
	void connecting(int socketfd);//��connect()��ʹ�� ʹ��connect()�����ɵ�socketfd Ϊ�µ����ӳ��Դ����µ�channel(socket_fd),����Ӧ��д�����ʹ���λ�Ķ���
	//linux �����׽���socket_fd �ڷ�����ģʽ�� ���ӽ�������ʧ�ܶ��ǿ�д�� ���ӽ���ҲҪ���Ǽ����������� ���Կ����ʵ�������
	void handle_write();//�������� �ж�ʧ�� ������ �ɹ�Ȼ�������Ӧ����
	void handle_error();//��ӡ�������� ���Է���������socket�Ĺر�
	void retry(int socketfd);//ʧ�� ���� ������socket�Ĺر� �����Ҫ ��ʱ������ʱ������������
	int remove_and_reset_channel();//channel_����ָ������� ��������ָ���channel Ϊʲô����������������channel��
	//����reset channel_֮��Ĳ���ȫ���  ��������ʱchannel�а󶨵Ļص�������һ���� ��������ִ�й�������������
	void reset_channel();//channel������ queue_in_loop
	
	event_loop* loop_;
	inet_address service_addr;
	std::unique_ptr<channel> channel_;
	state state_;
	bool connect_;//�����Ƿ�������õ�ַ
	new_connection_callback new_connection_callback_;
	int retry_delay_ms;//
	//��ʱ�����
	timer_queue::timer_weak timer_;
	
};