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
	void on_collision_x( const GridCell& ) { }
	void on_collision_y( const GridCell& ) { }
	void on_collision_z( const GridCell& ) { }

	float get_collision_width() const { return 2.f; }
	float get_collision_height() const { return 2.f; }
	float get_collision_depth() const { return 2.f; }

public:
	Medal();
	~Medal() { }

	/// 更新
	void update();

	void restart();
	
}; // class Medal

} // namespace blue_sky

#endif // BLUE_SKY_MEDAL_H
