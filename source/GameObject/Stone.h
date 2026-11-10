#pragma once

#include "ActiveObject.h"

namespace blue_sky
{

/**
 * 石
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

	static Vector GravityDefault;
	const Vector& get_default_gravity() const override { return GravityDefault; }

	/// 相手には自分を EntityId で渡す（相手は石の型を知らない）
	void on_collide_with( GameObject* o ) override;
	void on_collide_with( Player* ) override;

public:
	Stone();
	~Stone() { }

	/// 更新
	void update() override;

	void restart() override;
	
}; // class Stone

} // namespace blue_sky
