#ifndef BLUE_SKY_ACTIVE_OBJECT_H
#define BLUE_SKY_ACTIVE_OBJECT_H

#include "vector3.h"
#include <game/AABB.h>
#include <list>

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
 * 行動するオブジェクト
 *
 */
class ActiveObject
{
public:
	typedef game::Sound Sound;
	typedef game::AABB< vector3 > AABB;
	typedef std::list< AABB > AABBList;

	enum Direction
	{
		FRONT = 0, RIGHT, BACK, LEFT,
		DIRECTION_MAX
	};

private:
	const Stage* stage_;					///< ステージへの参照

	vector3		position_;					///< 座標
	vector3		velocity_;					///< 移動量

	Direction	direction_;					///< 方向
	float		direction_degree_;			///< 方向 ( Y Axis Degree )

	vector3		front_;						///< 前
	vector3		right_;						///< 右

	AABBList	local_aabb_list_;			///< AABB List ( local )
	AABBList	global_aabb_list_;			///< AABB List ( global )
	

	vector3		start_position_;			///< ゲーム開始時の座標
	float		start_direction_degree_;	///< ゲーム開始時の方向

	bool		is_dead_;					///< 死亡フラグ

protected:
	const Stage* stage() const { return stage_; }

	AABBList& local_aabb_list() { return local_aabb_list_; }

	float get_max_speed() const { return 1.f; }
	virtual float get_collision_width() const;
	virtual float get_collision_height() const;
	virtual float get_collision_depth() const;
	
	const GridCell& get_floor_cell() const;

	void limit_velocity();
	void update_position();	
	void update_global_aabb_list();

	virtual void on_collision_x( const GridCell& ) { }
	virtual void on_collision_y( const GridCell& ) { }
	virtual void on_collision_z( const GridCell& ) { }

	void play_sound( const char*, bool = false, bool = true ) const;
	void stop_sound( const char* ) const;

public:
	ActiveObject();
	~ActiveObject() { }
	
	void set_stage( const Stage* );	

	vector3& position() { return position_; }
	const vector3& position() const { return position_; }

	vector3& velocity() { return velocity_; }
	const vector3& velocity() const { return velocity_; }

	Direction direction() const { return direction_; }

	float get_direction_degree() const { return direction_degree_; }
	void set_direction_degree( float d );

	vector3& front() { return front_; }
	const vector3& front() const { return front_; }

	vector3& right() { return right_; }
	const vector3& right() const { return right_; }

	const AABBList& local_aabb_list() const { return local_aabb_list_; }
	const AABBList& global_aabb_list() const { return global_aabb_list_; }

	vector3& start_position() { return start_position_; }
	void set_start_direction_degree( float d ) { start_direction_degree_ = d; }

	void limit_position();

	virtual void kill();
	virtual void restart();

	/// 更新
	virtual void update() = 0;

	bool collision_detection( const ActiveObject* ) const;

	void render_shadow();

	const GridCell& get_floor_cell_center() const;
	const GridCell& get_floor_cell_left_front() const;
	const GridCell& get_floor_cell_right_front() const;
	const GridCell& get_floor_cell_left_back() const;
	const GridCell& get_floor_cell_right_back() const;
	
	bool is_dead() const { return is_dead_; }

}; // class ActiveObject

} // namespace blue_sky

#endif // BLUE_SKY_ACTIVE_OBJECT_H
