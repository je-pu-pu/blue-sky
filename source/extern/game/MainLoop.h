#ifndef GAME_MAIN_LOOP_H
#define GAME_MAIN_LOOP_H

namespace game
{

/**
 * メインループ管理クラス
 * 
 */
class MainLoop
{
private:
	unsigned long fps_;				// 目標 FPS
	unsigned long last_fps_;		// 前の 1 秒間の FPS
	unsigned long current_fps_;		// 現在の 1 秒間の FPS

	bool is_active_;				// アクティブフラグ

	unsigned long last_time_;		// 前のフレームの時間
	unsigned long current_time_;	// 現在のフレームの時間
	unsigned long last_sec_time_;	// 前の 1 秒の時間

public:
	MainLoop( unsigned long );
	~MainLoop();

	void reset();

	bool loop();
	
	unsigned long get_last_time() const { return last_time_; }
	unsigned long get_current_time() const { return current_time_; }
	
	float get_elapsed_sec() const { return get_elapsed_msec() / 1000.f; }
	unsigned long get_elapsed_msec() const { return current_time_ - last_time_; }
	
	unsigned long get_fps() const { return fps_; }
	unsigned long get_last_fps() const { return last_fps_; }
}; // class MainLoop

} // game

#endif // GAME_MAIN_LOOP_H