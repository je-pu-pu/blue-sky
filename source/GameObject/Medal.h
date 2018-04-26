#pragma once

#include "ActiveObject.h"

namespace blue_sky
{

/**
 * ���_��
 *
 */
class Medal : public ActiveObject
{
public:

private:
	float_t get_collision_width() const { return 2.f; }
	float_t get_collision_height() const { return 2.f; }
	float_t get_collision_depth() const { return 2.f; }

	float_t get_height_offset() const { return 0.f; }

	void on_collide_with( GameObject* o ) { o->on_collide_with( this ); }

	bool is_hard() const { return false; }

public:
	Medal();
	~Medal() { }

	/// �X�V
	void update();

	void restart();
	
}; // class Medal

} // namespace blue_sky
