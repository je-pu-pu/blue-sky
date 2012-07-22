#ifndef BLUE_SKY_STATIC_OBJECT_H
#define BLUE_SKY_STATIC_OBJECT_H

#include "ActiveObject.h"
#include "type.h"

namespace blue_sky
{

/**
 * �ړ����Ȃ��I�u�W�F�N�g
 *
 */
class StaticObject : public ActiveObject
{
private:
	float_t		collision_width_;
	float_t		collision_height_;
	float_t		collision_depth_;

protected:

public:
	StaticObject( float_t = 0, float_t = 0, float_t = 0 );
	virtual ~StaticObject();

	/// �X�V
	virtual void update() { }
	virtual void update_transform();
	
	virtual float get_collision_width() const { return collision_width_; }
	virtual float get_collision_height() const { return collision_height_; }
	virtual float get_collision_depth() const { return collision_depth_; }

}; // class StaticObject

} // namespace blue_sky

#endif // BLUE_SKY_STATIC_OBJECT_H
