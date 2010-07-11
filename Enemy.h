#ifndef BLUE_SKY_ENEMY_H
#define BLUE_SKY_ENEMY_H

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
 * 敵
 *
 */
class Enemy
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
	const Stage* stage_;			///< ステージへの参照

	vector3		position_;			///< 座標
	vector3		velocity_;			///< 移動量

	Direction	direction_;			///< 方向
	float		direction_degree_;	///< 方向 ( Degree )

	vector3		front_;				///< 前
	vector3		right_;				///< 右
	
	AABB		aabb_;				///< AABB

	bool		is_dead_;			///< 死亡フラグ
	bool		is_turn_avaiable_;	///< 方向転換有効フラグ
	bool		is_jumping_;		///< ジャンプ中フラグ
	bool		is_clambering_;		///< よじ登り中フラグ

	bool		is_falling_;		///< 落下中フラグ
	vector3		velocity_on_fall_;	///< 落下開始時の移動量
	
	float get_max_speed();
	float get_collision_width() const;
	float get_collision_height() const;
	float get_collision_depth() const;
	
	const GridCell& get_floor_cell() const;

	void play_sound( const char*, bool = false ) const;
	void stop_sound( const char* ) const;

public:
	Enemy();
	~Enemy() { }

	/// 更新
	void update();
	
}; // class Enemy

} // namespace blue_sky

#endif // BLUE_SKY_ENEMY_H
