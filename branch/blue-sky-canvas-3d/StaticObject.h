#ifndef BLUE_SKY_STATIC_OBJECT_H
#define BLUE_SKY_STATIC_OBJECT_H

#include "ActiveObject.h"
#include "type.h"

namespace blue_sky
{

/**
 * 移動しないオブジェクト
 *
 */
class StaticObject : public ActiveObject
{
private:
	float_t		collision_width_;
	float_t		collision_height_;
	float_t		collision_depth_;

	float get_collision_width() const { return collision_width_; }
	float get_collision_height() const { return collision_height_; }
	float get_collision_depth() const { return collision_depth_; }

	void on_collide_with( ActiveObject* o ) { }

protected:

public:
	StaticObject( float_t = 0, float_t = 0, float_t = 0 );
	~StaticObject();

	/// 更新
	void update() { }
	void update_transform();

}; // class StaticObject

} // namespace blue_sky

#endif // BLUE_SKY_STATIC_OBJECT_H
