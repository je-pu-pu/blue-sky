#ifndef GAME_ELAPSED_TIMER_H
#define GAME_ELAPSED_TIMER_H

namespace game
{

class MainLoop;

/**
 * 経過時間計計測
 * 
 * このクラスは MainLoop を参照し、経過時間を計測する
 */
class ElapsedTimer
{
private:
	const MainLoop* main_loop_;

	unsigned long last_time_;		// 前のフレームの時間
	unsigned long current_time_;	// 現在のフレームの時間

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