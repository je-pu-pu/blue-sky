#pragma once

#include "BaseSwitch.h"

namespace blue_sky
{

/**
 * 空間スイッチ
 *
 */
class AreaSwitch : public BaseSwitch
{
public:

private:
	float_t width_;
	float_t height_;
	float_t depth_;

	bool is_hard() const override { return false; }
	bool is_block() const override { return false; }
	bool is_ghost() const override { return true; }

	float get_collision_width()  const override { return width_; }
	float get_collision_height() const override { return height_; }
	float get_collision_depth()  const override { return depth_; }

	float_t get_height_offset() const override { return 0.f; }

	void on_collide_with( Stone* ) override { }

public:
	AreaSwitch( float_t, float_t, float_t );
	~AreaSwitch() { }

	/// 更新
	void update() override;

	void restart() override;
	
}; // class Switch

} // namespace blue_sky
