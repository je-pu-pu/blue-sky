#ifndef BLUE_SKY_BALLOON_H
#define BLUE_SKY_BALLOON_H

#include "ActiveObject.h"

namespace blue_sky
{

/**
 * ïóëD
 *
 */
class Balloon : public ActiveObject
{
public:
	const Player* player_;
	float flicker_;

private:
	float_t get_collision_width() const { return 1.5f; }
	float_t get_collision_height() const { return 2.f; }
	float_t get_collision_depth() const { return 1.5f; }
	
	float_t get_height_offset() const { return 0.f; }

	void on_collide_with( ActiveObject* o ) { o->on_collide_with( this ); }

	bool is_hard() const { return false; }
	bool is_balloon() const { return true; }

public:
	Balloon();
	~Balloon() { }

	/// çXêV
	void update();

	void restart();
	
	void set_player( const Player* p ) { player_ = p; flicker_ = 0.f; }
	const Player* get_player() const { return player_; }

}; // class Balloon

} // namespace blue_sky

#endif // BLUE_SKY_BALLOON_H
