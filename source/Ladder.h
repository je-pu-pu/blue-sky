#ifndef BLUE_SKY_LADDER_H
#define BLUE_SKY_LADDER_H

#include "ActiveObject.h"
#include <game/TimedCache.h>

namespace blue_sky
{

/**
 * ’òŽq
 *
 */
class Ladder : public ActiveObject
{
public:
	typedef game::TimedCache< const GameObject* > CntactObjectCache;

private:
	CntactObjectCache contact_object_cache_;

	bool is_hard() const override { return false; }

	void on_collide_with( GameObject* o ) override { o->on_collide_with( this ); }
	void on_collide_with( Player* ) override;

public:
	Ladder() { }
	~Ladder() { }

	void restart() override;
	void update() override;
	
	float_t get_collision_width() const override { return 0.4f; }
	float_t get_collision_height() const override { return 5.f; }
	float_t get_collision_depth() const override { return 0.2f; }

}; // class Ladder

} // namespace blue_sky

#endif // BLUE_SKY_LADDER_H
