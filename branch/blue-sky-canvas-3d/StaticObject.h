#ifndef BLUE_SKY_STATIC_OBJECT_H
#define BLUE_SKY_STATIC_OBJECT_H

#include "ActiveObject.h"

namespace blue_sky
{

/**
 * 移動しないオブジェクト
 *
 */
class StaticObject : public ActiveObject
{
private:

protected:

public:
	StaticObject();
	virtual ~StaticObject();

	/// 更新
	virtual void update() { };
	
	virtual float get_collision_width() const { return 0.f; }
	virtual float get_collision_height() const { return 0.f; }
	virtual float get_collision_depth() const { return 0.f; }

}; // class StaticObject

} // namespace blue_sky

#endif // BLUE_SKY_STATIC_OBJECT_H
