#pragma once

#include "ActiveObject.h"

namespace blue_sky
{

/**
 * �S�[���̔�
 *
 */
class Goal : public ActiveObject
{
public:


private:
	float get_collision_width() const override { return 4.f; }
	float get_collision_height() const override { return 2.f; }
	float get_collision_depth() const override { return 4.f; }

	const Vector& get_default_gravity() const override { return Vector::Zero; }

	void on_collide_with( GameObject* ) override { }

	bool is_safe_footing() const override { return true; }

	void on_arrive_at_target_location() override
	{
		set_velocity( Vector::Zero );
	}

public:
	Goal();
	~Goal() { }

	void restart();

	/// �X�V
	void update();
	
}; // class Goal

} // namespace blue_sky
