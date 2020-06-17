#pragma once

#include "ActiveObject.h"

namespace blue_sky
{

class Player;

/**
 * 少女
 *
 */
class Girl : public ActiveObject
{
public:

	enum class Mode
	{
		STAND = 0,
		FLOAT,
	};

private:
	Mode			mode_ = Mode::STAND;			///< 現在の動作モード
	Vector			flicker_base_location_;

	float get_collision_width() const override { return 0.5f; }
	float get_collision_height() const override { return 1.5f; }
	float get_collision_depth() const override { return 0.5f; }

	void on_collide_with( GameObject* o ) override { o->on_collide_with( this ); }

	const Vector& get_default_gravity() const override { return Vector::Zero; }

	bool is_hard() const override { return true; }

	void on_arrive_at_target_location() override
	{
		set_velocity( Vector::Zero );
		flicker_base_location_ = get_location();
	}

protected:
	const Player* player_ = nullptr;

public:
	Girl();
	~Girl() { }

	DEFINE_ACTIVE_OBJECT_COMMON_FUNCTIONS

	void set_player( const Player* p ) { player_ = p; }

	void restart() override;

	/// 更新
	void update() override;

	void action( const string_t& ) override;

	bool is_visible() const override { return true; }

}; // class Girl

} // namespace blue_sky
