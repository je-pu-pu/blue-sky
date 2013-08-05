#include "ElapsedTimer.h"
#include "MainLoop.h"

namespace game
{

ElapsedTimer::ElapsedTimer( const MainLoop* main_loop )
	: main_loop_( main_loop )
{
	reset();
}

/**
 * ���Z�b�g
 *
 *
 */
void ElapsedTimer::reset()
{
	last_time_ = main_loop_->get_current_time_force();
	current_time_ = last_time_;
}

/**
 * �X�V
 *
 * MainLoop ���猻�݂̎��Ԃ��擾���A�o�ߎ��Ԃ��v�Z�ł���悤�ɂ���
 */
void ElapsedTimer::update()
{
	last_time_ = current_time_;
	current_time_ = main_loop_->get_current_time();
}

} // namespace game