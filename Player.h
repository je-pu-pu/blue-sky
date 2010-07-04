#ifndef BLUE_SKY_PLAYER_H
#define BLUE_SKY_PLAYER_H

#include "vector3.h"

namespace blue_sky
{

class Stage;
class GridCell;

/**
 * �v���C���[
 *
 */
class Player
{
public:
	enum Direction
	{
		FRONT = 0, RIGHT, BACK, LEFT,
		DIRECTION_MAX
	};

private:
	const Stage* stage_;			///< �X�e�[�W�ւ̎Q��

	vector3		position_;			///< ���W
	vector3		velocity_;			///< �ړ���

	Direction	direction_;			///< ����
	float		direction_degree_;	///< ���� ( Degree )

	vector3		front_;				///< �O
	vector3		right_;				///< �E
	
	bool		is_turn_avaiable_;	///< �����]���L���t���O
	bool		is_jumping_;		///< �W�����v���t���O

	bool		is_falling_;		///< �������t���O
	vector3		velocity_on_fall_;	///< �����J�n���̈ړ���
	
	float get_max_speed();
	float get_collision_width() const;
	float get_collision_depth() const;
	
	const GridCell& get_floor_cell() const;

public:
	Player();
	~Player() { }
	
	vector3& position() { return position_; }
	const vector3& position() const { return position_; }

	vector3& velocity() { return velocity_; }
	const vector3& velocity() const { return velocity_; }

	Direction direction() const { return direction_; }
	float direction_degree() const { return direction_degree_; }

	vector3& front() { return front_; }
	vector3& right() { return right_; }

	void step( float );
	void side_step( float );
	void turn( int );

	bool is_turn_available() const { return is_turn_avaiable_; }

	float get_step_speed() const { return 0.004f; }
	float get_side_step_speed() const { return 0.004f; }

	/// �X�V
	void update();
	
	/// �W�����v����
	void jump();
	bool is_jumping() const { return is_jumping_; }

	/// ��������
	void fall();
	bool is_falling() const { return is_falling_; }

	const GridCell& get_floor_cell_center() const;
	const GridCell& get_floor_cell_left_front() const;
	const GridCell& get_floor_cell_right_front() const;
	const GridCell& get_floor_cell_left_back() const;
	const GridCell& get_floor_cell_right_back() const;

	void set_stage( const Stage* );
	
}; // class Player

} // namespace blue_sky

#endif // BLUE_SKY_PLAYER_H
