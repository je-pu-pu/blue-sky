#include "MainLoop.h"

#include <common/log.h>

#include <windows.h>

#include <iostream>

#pragma comment ( lib, "winmm.lib" )

namespace game
{

MainLoop::MainLoop( unsigned long fps )
	: fps_( fps )
	, current_fps_( 0 )
	, last_fps_( 0 )
	, is_active_( true )
	, last_time_( timeGetTime() )
	, current_time_( last_time_ )
	, last_sec_time_( last_time_ )
{
	timeBeginPeriod( 1 );
}

MainLoop::~MainLoop()
{
	timeEndPeriod( 1 );
}

/**
 * リセット
 *
 *
 */
void MainLoop::reset()
{
	last_time_ = timeGetTime();
	current_time_ = last_time_;
	last_sec_time_ = last_time_;
}

/**
 * ループ
 *
 * 指定された時間が経過していればtureを返す
 */
bool MainLoop::loop()
{
	// アクティブでなければ終了
	if ( ! is_active_ )
	{
		return false;
	}

	// 現在の時間を取得
	unsigned long time = timeGetTime();

	// 指定された時間が経過したかチェック
	if ( time - last_sec_time_ >= 1000 * current_fps_ / fps_ )
	{
		if ( time - last_sec_time_ >= 1000 )
		{
			last_fps_ = current_fps_;
			current_fps_ = 0;
			last_sec_time_ = time;

			// common::log( "log/elapsed.log", "---\n" );
		}
		
		current_fps_++;
		
		//
		last_time_ = current_time_;
		current_time_ = fixed_elapsed_msec_ ? current_time_ + fixed_elapsed_msec_ : time;

		return true;
	}

	return false;
}

unsigned long MainLoop::get_wait_msec() const
{
	if ( ! is_active_ )
	{
		return 0;
	}

	// loop() が true を返す条件「経過 >= 1000 * current_fps_ / fps_」を満たす時刻までの残り
	const unsigned long elapsed = timeGetTime() - last_sec_time_;
	const unsigned long next = 1000 * current_fps_ / fps_;

	return elapsed >= next ? 0 : next - elapsed;
}

/**
 * 現在の時間を取得する
 *
 * get_current_time() は MainLoop に保存されたキャッシュを返すが、
 * この関数は、本当の現在時間を再取得する。
 */
unsigned long MainLoop::get_current_time_force() const
{
	return timeGetTime();
}

} // namespace game