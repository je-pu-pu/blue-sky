#pragma once

#include "ActiveObject.h"

namespace blue_sky
{

/**
 * P
 *
 */
class Umbrella : public ActiveObject
{
public:
	float flicker_;

private:
	bool is_hard() const override { return false; }

	float get_collision_width() const override { return 2.f; }
	float get_collision_height() const override { return 2.f; }
	float get_collision_depth() const override { return 2.f; }

	void on_collide_with( GameObject* o ) override { o->on_collide_with( this ); }
	void on_collide_with( Player* ) override;

public:
	Umbrella();
	~Umbrella() { }

	/// XV
	void update() override;

	void restart() override;
	
}; // class Umbrella

} // namespace blue_sky
