#ifndef BLUE_SKY_GIRL_H
#define BLUE_SKY_GIRL_H

#include "ActiveObject.h"

namespace blue_sky
{

class Player;

/**
 * ­—
 *
 */
class Girl : public ActiveObject
{
public:

private:
	float_t flicker_;

	float get_collision_width() const { return 1.f; }
	float get_collision_height() const { return 1.f; }
	float get_collision_depth() const  { return 1.f; }

	void on_collide_with( GameObject* o ) { o->on_collide_with( this ); }

protected:


public:
	Girl();
	~Girl() { }

	void restart();

	/// XV
	void update();

	bool is_visible() const { return true; }

}; // class Girl

} // namespace blue_sky

#endif // BLUE_SKY_GIRL_H
