#ifndef BLUE_SKY_PLAYER_H
#define BLUE_SKY_PLAYER_H

#include "vector3.h"
#include <game/AABB.h>

namespace game
{

class Sound;
template< typename > class AABB;

} // namespace game

namespace blue_sky
{

class Input;
class Stage;
class GridCell;

/**
 * �v���C���[
 *
 */
class Player
{
public:
	typedef game::Sound Sound;
	typedef game::AABB< vector3 > AABB;

	enum Direction
	{
		FRONT = 0, RIGHT, BACK, LEFT,
		DIRECTION_MAX
	};

private:
	const Input* input_;			///< ���͂ւ̎Q��
	const Stage* stage_;			///< �X�e�[�W�ւ̎Q��

	vector3		position_;			///< ���W
	vector3		velocity_;			///< �ړ���

	Direction	direction_;			///< ����
	float		direction_degree_;	///< ���� ( Y Axis Degree )

	float		step_speed_;		///< �ړ����x

	vector3		front_;				///< �O
	vector3		right_;				///< �E
	float		eye_height_;		///< �ڂ̍���

	AABB		aabb_;				///< AABB

	bool		is_dead_;			///< ���S�t���O
	bool		is_turn_avaiable_;	///< �����]���L���t���O
	bool		is_jumping_;		///< �W�����v���t���O
	bool		is_clambering_;		///< �悶�o�蒆�t���O

	bool		is_falling_;		///< �������t���O
	vector3		velocity_on_fall_;	///< �����J�n���̈ړ���
	
	float get_max_speed() const;
	float get_collision_width() const;
	float get_collision_height() const;
	float get_collision_depth() const;
	
	const GridCell& get_floor_cell() const;

	void play_sound( const char*, bool = false ) const;
	void stop_sound( const char* ) const;

public:
	Player();
	~Player() { }
	
	vector3& position() { return position_; }
	const vector3& position() const { return position_; }

	vector3& velocity() { return velocity_; }
	const vector3& velocity() const { return velocity_; }

	Direction direction() const { return direction_; }

	float direction_degree() const { return direction_degree_; }
	void add_direction_degree( float );

	vector3& front() { return front_; }
	vector3& right() { return right_; }

	float get_eye_height() const { return eye_height_; }

	AABB& aabb() { return aabb_; }

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

	const GridCell& get_floor_cell_center() const;
	const GridCell& get_floor_cell_left_front() const;
	const GridCell& get_floor_cell_right_front() const;
	const GridCell& get_floor_cell_left_back() const;
	const GridCell& get_floor_cell_right_back() const;

	void set_input( const Input* );
	void set_stage( const Stage* );	
	
}; // class Player

} // namespace blue_sky

#endif // BLUE_SKY_PLAYER_H
