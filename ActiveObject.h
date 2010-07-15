#ifndef BLUE_SKY_ACTIVE_OBJECT_H
#define BLUE_SKY_ACTIVE_OBJECT_H

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
 * �s������I�u�W�F�N�g
 *
 */
class ActiveObject
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
	const Stage* stage_;			///< �X�e�[�W�ւ̎Q��

	vector3		position_;			///< ���W
	vector3		velocity_;			///< �ړ���

	Direction	direction_;			///< ����
	float		direction_degree_;	///< ���� ( Y Axis Degree )

	vector3		front_;				///< �O

	AABB		aabb_;				///< AABB
	
protected:
	const Stage* stage() const { return stage_; }

	float get_max_speed() const { return 1.f; }
	virtual float get_collision_width() const;
	virtual float get_collision_height() const;
	virtual float get_collision_depth() const;
	
	const GridCell& get_floor_cell() const;

	void limit_velocity();
	void update_position();

	virtual void on_collision_x( const GridCell& ) { }
	virtual void on_collision_y( const GridCell& ) { }
	virtual void on_collision_z( const GridCell& ) { }

	void play_sound( const char*, bool = false ) const;
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

	AABB& aabb() { return aabb_; }

	/// �X�V
	virtual void update() = 0;

	void render_shadow();

	const GridCell& get_floor_cell_center() const;
	const GridCell& get_floor_cell_left_front() const;
	const GridCell& get_floor_cell_right_front() const;
	const GridCell& get_floor_cell_left_back() const;
	const GridCell& get_floor_cell_right_back() const;
	
}; // class ActiveObject

} // namespace blue_sky

#endif // BLUE_SKY_ACTIVE_OBJECT_H
