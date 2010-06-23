#ifndef BLUE_SKY_PLAYER_H
#define BLUE_SKY_PLAYER_H

#include "vector3.h"

namespace blue_sky
{

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
	vector3		position_;		///< ���W
	vector3		velocity_;		///< �ړ�

	Direction	direction_;		///< ����
	vector3		front_;			///< �O
	vector3		right_;			///< �E
	
	bool		is_jumping_;	///< �W�����v���t���O
	float		floor_height_;	///< ����̍���
	
	float max_speed();
public:
	Player();
	~Player() { }
	
	vector3& position() { return position_; }
	vector3& velocity() { return velocity_; }

	Direction direction() const { return direction_; }
	vector3& front() { return front_; }
	vector3& right() { return right_; }

	void step( float );
	void side_step( float );
	void turn( int );

	float get_step_speed() const { return 0.002f; }
	float get_side_step_speed() const { return 0.002f; }

	/// �X�V
	void update();
	
	/// �W�����v����
	void jump();

	bool is_jumping() { return is_jumping_; }

	float get_floor_height() const { return floor_height_; }
	void set_floor_height( float h ) { floor_height_ = h; }
	
}; // class Player

} // namespace blue_sky

#endif // BLUE_SKY_PLAYER_H
