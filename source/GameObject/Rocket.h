#pragma once

#include "ActiveObject.h"

namespace blue_sky
{

/**
 * ロケット
 *
 */
class Rocket : public ActiveObject
{
private:
	bool is_hard() const override { return true; }
	bool is_block() const override { return false; }

	float get_collision_width() const override { return 0.2f; }
	float get_collision_height() const override { return 1.f; }
	float get_collision_depth() const override { return 0.2f; }

	void on_collide_with( GameObject* o ) override { o->on_collide_with( this ); }
	void on_collide_with( Player* ) override;

public:
	Rocket();
	~Rocket() { }

	/// 更新
	void update() override;

	void restart() override;
	
}; // class Rocket

} // namespace blue_sky
