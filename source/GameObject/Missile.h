#pragma once

#include "ActiveObject.h"

namespace blue_sky
{

/**
 * 敵ロボットが発射するミサイル
 *
 */
class Missile : public ActiveObject
{
public:

private:
	private const GameObject* target_;

	void on_collide_with( GameObject* o ) { o->on_collide_with( this ); }
	void on_collide_with( Player* );

protected:

public:
	Missile();
	~Missile() { }

	void set_target( const GameObject* o ) { target_ = o; }

	void restart();

	/// 更新
	void update();

	Mode get_mode() const { return mode_; };
	
	float get_collision_width() const { return 1.f; }
	float get_collision_height() const { return 2.f; }
	float get_collision_depth() const  { return 0.5f; }

}; // class Robot

} // namespace blue_sky
