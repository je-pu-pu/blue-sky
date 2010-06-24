#ifndef BLUE_SKY_PLAYER_H
#define BLUE_SKY_PLAYER_H

#include "vector3.h"

namespace blue_sky
{

class Stage;

/**
 * プレイヤー
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
	vector3		position_;			///< 座標
	vector3		velocity_;			///< 移動

	Direction	direction_;			///< 方向
	float		direction_degree_;	///< 方向 ( Degree )

	vector3		front_;				///< 前
	vector3		right_;				///< 右
	
	bool		is_jumping_;		///< ジャンプ中フラグ
	
	const Stage* stage_;			///< ステージ
	
	float get_max_speed();
	float get_collision_width() const;
	float get_collision_depth() const;
	float get_floor_height() const;

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

	float get_step_speed() const { return 0.002f; }
	float get_side_step_speed() const { return 0.002f; }

	/// 更新
	void update();
	
	/// ジャンプ処理
	void jump();

	bool is_jumping() const { return is_jumping_; }

	float get_floor_height_center() const;
	float get_floor_height_left_front() const;
	float get_floor_height_right_front() const;
	float get_floor_height_left_back() const;
	float get_floor_height_right_back() const;

	void set_stage( const Stage* );
	
}; // class Player

} // namespace blue_sky

#endif // BLUE_SKY_PLAYER_H
