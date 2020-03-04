#pragma once
#include <muduo/base/Timestamp.h>

#include <functional>
#include <memory>
//timer���¼��ʱ������Ч�ڣ�������ʽ��ѭ�����ڣ��ص�����
class timer:public std::enable_shared_from_this<timer>
{
public:
	friend struct less_compare;
	typedef std::function<void(void)> callback;
	timer(const muduo::Timestamp& delay,double interval_time,const callback& cb);
	void run();
	//����ѭ����־λ��Ϊfalse ������Ϊ�� ʹ֮���굱ǰ��ѭ��������
	void set_invalid();
	bool restart();
	bool check(muduo::Timestamp time_stamp) const;
	muduo::Timestamp waiting_time() const;
private:
	static const callback invaild_cb_ ;
	muduo::Timestamp waiting_time_;
	double interval;
	bool repeat;
	callback cb_;
};
struct less_compare
{
	bool operator()(const std::shared_ptr<timer>& timer_1, const std::shared_ptr<timer>& timer_2);
};
