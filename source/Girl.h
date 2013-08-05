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

	float get_collision_width() const override { return 0.5f; }
	float get_collision_height() const override { return 1.5f; }
	float get_collision_depth() const override { return 0.5f; }

	void on_collide_with( GameObject* o ) override { o->on_collide_with( this ); }

	const Vector3& get_default_gravity() const override { return GravityZero; }

	bool is_hard() const override { return true; }

protected:
	const Player* player_;

public:
	Girl();
	~Girl() { }

	void set_player( const Player* p ) { player_ = p; }

	void restart() override;

	/// XV
	void update() override;

	bool is_visible() const override { return true; }

}; // class Girl

} // namespace blue_sky

#endif // BLUE_SKY_GIRL_H
