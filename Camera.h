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
	vector3 position_;		///< ���_
	vector3 front_;			///< �O
	vector3 front_target_;	///< �O ( �ڕW )
	vector3 look_at_;		///< �����_
	vector3 up_;			///< ��

	float fov_;			///< FOV
	float under_view_rate_;	///< �^�����_�̓x���� ( 0.f .. 1.f )
public:
	Camera();
	~Camera();

	vector3& position() { return position_; }
	const vector3& look_at() { return look_at_; }
	const vector3& up() const { return up_; }

	void set_front_target( const vector3& v ) { front_target_ = v; }

	float& fov() { return fov_; }

	void update();

	void set_under_view_rate( float );
	float get_under_view_rate() const { return under_view_rate_; }
};

} // namespace blue_sky

#endif // BLUE_SKY_CAMERA_H