#ifndef BLUE_SKY_CAMERA_H
#define BLUE_SKY_CAMERA_H

#include "vector3.h"

namespace blue_sky
{

/**
 * �J����
 *
 */
class Camera
{
public:
	
private:
	vector3 position_;	///< ���_
	vector3 look_at_;	///< �����_
	vector3 up_;		///< ��

public:
	Camera();
	~Camera();

	vector3& position() { return position_; }
	vector3& look_at() { return look_at_; }
	vector3& up() { return up_; }
};

} // namespace blue_sky

#endif // BLUE_SKY_CAMERA_H