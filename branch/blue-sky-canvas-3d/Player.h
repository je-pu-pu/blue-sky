#ifndef BLUE_SKY_PLAYER_H
#define BLUE_SKY_PLAYER_H

#include "ActiveObject.h"

namespace blue_sky
{

/**
 * �v���C���[
 *
 */
class Player : public ActiveObject
{
public:

private:
	bool is_jumping_;			///< ���݃W�����v�\�t���O
	bool is_jumpable_;			///< ���݃W�����v���t���O

	float_t uncontrollable_timer_;	///< ����s�\�^�C�}�[

	float_t eye_height_;		///< �ڂ̍���

	float_t get_collision_width() const { return 0.5f; }
	float_t get_collision_height() const { return 1.75f; }
	float_t get_collision_depth() const { return 0.25f; }

protected:
	const Input* get_input() const;

	float_t get_step_speed() const { return 0.25f; }
	float_t get_side_step_speed() const { return 0.25f; }

	void update_jumpable();
	void update_jumping();

	bool is_jumpable() const { return is_jumpable_; }
	bool is_on_ground( float_t, float_t ) const;
	bool is_jumping() const { return is_jumping_; }

	bool is_uncontrollable() const { return uncontrollable_timer_ > 0.f; }

public:
	Player();
	~Player() { }

	void restart();

	/// �X�V
	void update();
	
	// �ړ�
	void step( float_t );
	void side_step( float_t );

	/// �W�����v����
	void jump();

	// ��~
	void stop();

	/// �������Z
	void add_direction_degree( float );

	void damage( const Vector3& );
	void kill();

	float_t get_eye_height() const { return eye_height_; }

	//
	bool is_falling_to_dead() const;

}; // class Player

} // namespace blue_sky

#endif // BLUE_SKY_PLAYER_H
