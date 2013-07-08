#ifndef GAME_MAIN_LOOP_H
#define GAME_MAIN_LOOP_H

namespace game
{

/**
 * ���C�����[�v�Ǘ��N���X
 * 
 */
class MainLoop
{
private:
	unsigned long fps_;				// �ڕW FPS
	unsigned long last_fps_;		// �O�� 1 �b�Ԃ� FPS
	unsigned long current_fps_;		// ���݂� 1 �b�Ԃ� FPS

	bool is_active_;				// �A�N�e�B�u�t���O

	unsigned long last_time_;		// �O�̃t���[���̎���
	unsigned long current_time_;	// ���݂̃t���[���̎���
	unsigned long last_sec_time_;	// �O�� 1 �b�̎���

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