#ifndef GAME_ELAPSED_TIMER_H
#define GAME_ELAPSED_TIMER_H

namespace game
{

class MainLoop;

/**
 * �o�ߎ��Ԍv�v��
 * 
 * ���̃N���X�� MainLoop ���Q�Ƃ��A�o�ߎ��Ԃ��v������
 */
class ElapsedTimer
{
private:
	const MainLoop* main_loop_;

	unsigned long last_time_;		// �O�̃t���[���̎���
	unsigned long current_time_;	// ���݂̃t���[���̎���

public:
	ElapsedTimer( const MainLoop* );
	~ElapsedTimer() { }
	
	void update();
	
	void reset();

	float get_elapsed_sec() const { return get_elapsed_msec() / 1000.f; }
	unsigned long get_elapsed_msec() const { return current_time_ - last_time_; }

}; // class MainLoop

} // game

#endif // GAME_MAIN_LOOP_H