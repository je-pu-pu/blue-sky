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
	const Input* input_;			///< ���͂ւ̎Q��

	float		step_speed_;		///< �ړ����x

	float		eye_height_;		///< �ڂ̍���

	bool		is_dead_;			///< ���S�t���O
	bool		is_turn_avaiable_;	///< �����]���L���t���O
	bool		is_jumping_;		///< �W�����v���t���O
	bool		is_clambering_;		///< �悶�o�蒆�t���O

	bool		is_falling_;		///< �������t���O
	vector3		velocity_on_fall_;	///< �����J�n���̈ړ���
	
	void on_collision_x( const GridCell& );
	void on_collision_y( const GridCell& );
	void on_collision_z( const GridCell& );

	float get_collision_width() const;
	float get_collision_height() const;
	float get_collision_depth() const;

public:
	Player();
	~Player() { }
	
	void add_direction_degree( float );

	float get_eye_height() const { return eye_height_; }

	void step( float );
	void side_step( float );

	bool is_dead() const { return is_dead_; }
	void rebirth();

	bool is_turn_available() const { return is_turn_avaiable_; }

	float get_step_speed() const { return step_speed_; }
	float get_side_step_speed() const { return 0.004f; }

	/// �X�V
	void update();
	
	/// �W�����v����
	void jump();
	bool is_jumping() const { return is_jumping_; }

	bool is_clambering() const { return is_clambering_; }

	/// ��������
	void fall();
	bool is_falling() const { return is_falling_; }
	bool is_falling_to_dead() const;

	void set_input( const Input* );
	
}; // class Player

} // namespace blue_sky

#endif // BLUE_SKY_PLAYER_H
