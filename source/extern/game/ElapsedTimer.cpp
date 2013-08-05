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
 * リセット
 *
 *
 */
void ElapsedTimer::reset()
{
	last_time_ = main_loop_->get_current_time_force();
	current_time_ = last_time_;
}

/**
 * 更新
 *
 * MainLoop から現在の時間を取得し、経過時間を計算できるようにする
 */
void ElapsedTimer::update()
{
	last_time_ = current_time_;
	current_time_ = main_loop_->get_current_time();
}

} // namespace game