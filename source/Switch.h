#ifndef BLUE_SKY_SWITCH_H
#define BLUE_SKY_SWITCH_H

#include "ActiveObject.h"

namespace blue_sky
{

/**
 * スイッチ
 *
 */
class Switch : public ActiveObject
{
public:
	enum State { ON = 0, OFF, BROKEN };

private:
	State state_;
	float_t state_switching_timer_; ///< 次にスイッチングが可能になるまでの時間

	bool is_hard() const override { return true; }
	bool is_block() const override { return true; }

	float get_collision_width()  const override { return 0.6f; }
	float get_collision_height() const override { return 0.6f; }
	float get_collision_depth()  const override { return 0.9f; }

	void on_collide_with( GameObject* o ) override { o->on_collide_with( this ); }
	void on_collide_with( Player* ) override;
	void on_collide_with( Stone* ) override;

public:
	Switch();
	~Switch() { }

	/// 更新
	void update() override;

	void restart() override;

	void do_switch();
	void do_break();
	
}; // class Switch

} // namespace blue_sky

#endif // BLUE_SKY_SWITCH_H
