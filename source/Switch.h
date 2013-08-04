#ifndef BLUE_SKY_SWITCH_H
#define BLUE_SKY_SWITCH_H

#include "BaseSwitch.h"

namespace blue_sky
{

/**
 * 通常スイッチ
 *
 */
class Switch : public BaseSwitch
{
public:

private:
	bool_t is_hard() const override { return true; }
	bool_t is_block() const override { return true; }

	float_t get_collision_width()  const override { return 0.6f; }
	float_t get_collision_height() const override { return 0.9f; }
	float_t get_collision_depth()  const override { return 0.6f; }

	void on_turn_on() override;
	void on_turn_off() override;
	void on_break() override;

public:
	Switch();
	~Switch() { }

	/// 更新
	void update() override;

	void restart() override;
	
}; // class Switch

} // namespace blue_sky

#endif // BLUE_SKY_SWITCH_H
