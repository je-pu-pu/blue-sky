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
 * �G
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
	const Stage* stage_;			///< �X�e�[�W�ւ̎Q��

	vector3		position_;			///< ���W
	vector3		velocity_;			///< �ړ���

	Direction	direction_;			///< ����
	float		direction_degree_;	///< ���� ( Degree )

	vector3		front_;				///< �O
	vector3		right_;				///< �E
	
	AABB		aabb_;				///< AABB

	bool		is_dead_;			///< ���S�t���O
	bool		is_turn_avaiable_;	///< �����]���L���t���O
	bool		is_jumping_;		///< �W�����v���t���O
	bool		is_clambering_;		///< �悶�o�蒆�t���O

	bool		is_falling_;		///< �������t���O
	vector3		velocity_on_fall_;	///< �����J�n���̈ړ���
	
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

	/// �X�V
	void update();
	
}; // class Enemy

} // namespace blue_sky

#endif // BLUE_SKY_ENEMY_H
