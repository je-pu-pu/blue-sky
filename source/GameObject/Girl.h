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

	enum Mode
	{
		MODE_STAND = 0,
		MODE_FLOAT,
	};

private:
	Mode			mode_;			///< 現在の動作モード
	Vector3			flicker_base_location_;

	float get_collision_width() const override { return 0.5f; }
	float get_collision_height() const override { return 1.5f; }
	float get_collision_depth() const override { return 0.5f; }

	void on_collide_with( GameObject* o ) override { o->on_collide_with( this ); }

	const Vector3& get_default_gravity() const override { return GravityZero; }

	bool is_hard() const override { return true; }

	void on_arrive_at_target_location() override
	{
		set_velocity( Vector3( 0, 0, 0 ) );
		flicker_base_location_ = get_location();
	}

protected:
	const Player* player_;

public:
	Girl();
	~Girl() { }

	void set_player( const Player* p ) { player_ = p; }

	void restart() override;

	/// 更新
	void update() override;

	void action( const string_t& ) override;

	bool is_visible() const override { return true; }

}; // class Girl

} // namespace blue_sky
