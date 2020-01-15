#include "event_loop.h"
#include "timer_queue.h"

void event_loop::run_at(Functor call_back, muduo::Timestamp time_stamp)
{
	timer timer_(time_stamp,0,call_back);
	timer_queue_->add_timer_in_loop(timer_);
}

void event_loop::run_every(Functor call_back, double interval_second)
{
	timer timer_(muduo::addTime(muduo::Timestamp::now(),0), interval_second, call_back);
	timer_queue_->add_timer_in_loop(timer_);
}

void event_loop::run_after(Functor call_back, double time_second)
{
	timer timer_(muduo::addTime(muduo::Timestamp::now(),time_second), 0, call_back);
	timer_queue_->add_timer_in_loop(timer_);
}

void event_loop::wake_up()
{
	eventfd_channel_->wake_up();
}
