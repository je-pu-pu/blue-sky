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
 * ���Z�b�g
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
 * ���[�v
 *
 * �w�肳�ꂽ���Ԃ��o�߂��Ă����ture��Ԃ�
 */
bool MainLoop::loop()
{
	// �A�N�e�B�u�łȂ���ΏI��
	if ( ! is_active_ )
	{
		return false;
	}

	// ���݂̎��Ԃ��擾
	unsigned long time = timeGetTime();

	// �w�肳�ꂽ���Ԃ��o�߂������`�F�b�N
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
		current_time_ = time;

		return true;
	}

	return false;
}

/**
 * ���݂̎��Ԃ��擾����
 *
 * get_current_time() �� MainLoop �ɕۑ����ꂽ�L���b�V����Ԃ����A
 * ���̊֐��́A�{���̌��ݎ��Ԃ��Ď擾����B
 */
unsigned long MainLoop::get_current_time_force() const
{
	return timeGetTime();
}

} // namespace game