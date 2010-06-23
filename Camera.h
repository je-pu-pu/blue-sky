#ifndef BLUE_SKY_CAMERA_H
#define BLUE_SKY_CAMERA_H

#include "vector3.h"

namespace blue_sky
{

/**
 * カメラ
 *
 */
class Camera
{
public:
	
private:
	vector3 position_;	///< 視点
	vector3 look_at_;	///< 注視点
	vector3 up_;		///< 上

	float fov_;			///< FOV
	float under_view_rate_;	///< 真下視点の度合い ( 0.f .. 1.f )
public:
	Camera();
	~Camera();

	vector3& position() { return position_; }
	vector3& look_at() { return look_at_; }
	const vector3& up() const { return up_; }

	float& fov() { return fov_; }

	void set_under_view_rate( float );
	float get_under_view_rate() const { return under_view_rate_; }
};

} // namespace blue_sky

#endif // BLUE_SKY_CAMERA_H