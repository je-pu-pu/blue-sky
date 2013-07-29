#ifndef BLUE_SKY_ROCKET_H
#define BLUE_SKY_ROCKET_H

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
	bool is_hard() const { return true; }

	float get_collision_width() const { return 0.2f; }
	float get_collision_height() const { return 1.f; }
	float get_collision_depth() const { return 0.2f; }

	void on_collide_with( GameObject* o ) { o->on_collide_with( this ); }
	void on_collide_with( Player* );

public:
	Rocket();
	~Rocket() { }

	/// 更新
	void update();

	void restart();
	
}; // class Rocket

} // namespace blue_sky

#endif // BLUE_SKY_ROCKET_H
