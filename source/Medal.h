#ifndef BLUE_SKY_MEDAL_H
#define BLUE_SKY_MEDAL_H

#include "ActiveObject.h"

namespace blue_sky
{

/**
 * メダル
 *
 */
class Medal : public ActiveObject
{
public:

private:
	float get_collision_width() const { return 2.f; }
	float get_collision_height() const { return 2.f; }
	float get_collision_depth() const { return 2.f; }

	float_t get_height_offset() const { return 0.f; }

	void on_collide_with( GameObject* o ) { o->on_collide_with( this ); }

public:
	Medal();
	~Medal() { }

	/// 更新
	void update();

	void restart();
	
}; // class Medal

} // namespace blue_sky

#endif // BLUE_SKY_MEDAL_H
