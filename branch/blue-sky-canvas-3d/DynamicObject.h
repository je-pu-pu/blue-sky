#ifndef BLUE_SKY_DYNAMIC_OBJECT_H
#define BLUE_SKY_DYNAMIC_OBJECT_H

#include "ActiveObject.h"
#include "type.h"

namespace blue_sky
{

/**
 * 移動するオブジェクト
 *
 */
class DynamicObject : public ActiveObject
{
private:
	float_t		collision_width_;
	float_t		collision_height_;
	float_t		collision_depth_;

	bool is_dynamic_object() const { return true; }

	void on_collide_with( ActiveObject* o ) { o->on_collide_with( this ); }

	void on_collide_with( Player* );
	void on_collide_with( StaticObject* );
	void on_collide_with_ground();

	void play_collision_sound( const ActiveObject* );

protected:

public:
	DynamicObject( float_t = 0, float_t = 0, float_t = 0 );
	virtual ~DynamicObject();

	/// 更新
	virtual void update() { }
	virtual void update_transform();
	
	virtual float get_collision_width() const { return collision_width_; }
	virtual float get_collision_height() const { return collision_height_; }
	virtual float get_collision_depth() const { return collision_depth_; }

}; // class StaticObject

} // namespace blue_sky

#endif // BLUE_SKY_DYNAMIC_OBJECT_H
