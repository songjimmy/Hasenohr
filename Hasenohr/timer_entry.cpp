#include "event_loop.h"
#include "timer_queue.h"

timer_queue::timer_weak event_loop::run_at(Functor call_back, muduo::Timestamp time_stamp)
{
	timer_queue::timer_ptr timer_=std::make_shared<timer>(time_stamp,0,call_back);
	timer_queue_->add_timer(timer_);
	return std::weak_ptr<timer>(timer_);
}

timer_queue::timer_weak event_loop::run_every(Functor call_back, double interval_second)
{
	timer_queue::timer_ptr timer_ = std::make_shared<timer>(muduo::addTime(muduo::Timestamp::now(),0), interval_second, call_back);
	timer_queue_->add_timer(timer_);
	return std::weak_ptr<timer>(timer_);
}

timer_queue::timer_weak event_loop::run_after(Functor call_back, double time_second)
{
	timer_queue::timer_ptr timer_ = std::make_shared<timer>(muduo::addTime(muduo::Timestamp::now(),time_second), 0, call_back);
	timer_queue_->add_timer(timer_);
	return std::weak_ptr<timer>(timer_);
}

void event_loop::wake_up()
{
	eventfd_channel_->wake_up();
}
