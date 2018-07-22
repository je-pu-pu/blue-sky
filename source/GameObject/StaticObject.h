#pragma once

#include "ActiveObject.h"
#include <type/type.h>

namespace blue_sky
{

/**
 * 移動しないオブジェクト
 *
 * ゲームの背景や障害物・小物などに使用する
 */
class StaticObject : public ActiveObject /// @todo GameObject を継承する
{
private:
	float_t		collision_width_;
	float_t		collision_height_;
	float_t		collision_depth_;
	float_t		mass_;

	float_t get_collision_width() const override { return collision_width_; }
	float_t get_collision_height() const override { return collision_height_; }
	float_t get_collision_depth() const override { return collision_depth_; }

	float_t get_default_mass() const override { return mass_; }

	void on_collide_with( GameObject* o ) override { o->on_collide_with( this ); }

protected:

public:
	StaticObject( float_t w = 0, float_t h = 0, float_t d = 0, float_t mass_ = 0.f );
	~StaticObject();

	/// 更新
	void update();

}; // class StaticObject

} // namespace blue_sky
