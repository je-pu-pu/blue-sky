#pragma once

#include "ActiveObject.h"

namespace blue_sky
{

/**
 * Î
 *
 */
class Stone : public ActiveObject
{
private:
	bool is_hard() const override { return true; }
	bool is_block() const override { return false; }

	float get_collision_width()  const override { return 0.2f; }
	float get_collision_height() const override { return 0.2f; }
	float get_collision_depth()  const override { return 0.2f; }

	static Vector3 GravityDefault;
	const Vector3& get_default_gravity() const override { return GravityDefault; }

	void on_collide_with( GameObject* o ) override { o->on_collide_with( this ); }
	void on_collide_with( Player* ) override;

public:
	Stone();
	~Stone() { }

	/// XV
	void update() override;

	void restart() override;
	
}; // class Stone

} // namespace blue_sky
