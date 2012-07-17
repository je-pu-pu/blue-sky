#ifndef BLUE_SKY_STATIC_OBJECT_H
#define BLUE_SKY_STATIC_OBJECT_H

#include "ActiveObject.h"

namespace blue_sky
{

/**
 * �ړ����Ȃ��I�u�W�F�N�g
 *
 */
class StaticObject : public ActiveObject
{
private:

protected:

public:
	StaticObject();
	virtual ~StaticObject();

	/// �X�V
	virtual void update() { };
	
	virtual float get_collision_width() const { return 0.f; }
	virtual float get_collision_height() const { return 0.f; }
	virtual float get_collision_depth() const { return 0.f; }

}; // class StaticObject

} // namespace blue_sky

#endif // BLUE_SKY_STATIC_OBJECT_H
