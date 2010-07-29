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
	vector3 position_;				///< 視点
	vector3 look_at_;				///< 注視点
	vector3 up_;					///< 上

	vector3 default_front_;			///< デフォルト視点
	vector3 default_up_;			///< デフォルト上

	vector3 front_;					///< 前

	vector3 rotate_degree_;			///< XYZ 各軸に対する回転角度
	vector3 rotate_degree_target_;	///< XYZ 各軸に対する回転角度 ( 目標 )

	float fov_;						///< FOV
	float rotate_chase_speed_;		///< 回転速度

public:
	Camera();
	~Camera();

	vector3& position() { return position_; }
	const vector3& look_at() { return look_at_; }
	const vector3& up() const { return up_; }

	const vector3& front() const { return front_; }

	vector3& rotate_degree() { return rotate_degree_; }
	vector3& rotate_degree_target() { return rotate_degree_target_; }

	const vector3& rotate_degree() const { return rotate_degree_; }
	const vector3& rotate_degree_target() const { return rotate_degree_target_; }

	const float& fov() const { return fov_; }
	void set_fov( float );

	float get_rotate_chase_speed() { return rotate_chase_speed_; }
	void set_rotate_chase_speed( float s ) { rotate_chase_speed_; }
	void reset_rotate_chase_speed() { rotate_chase_speed_ = 0.5f; }

	void update();

	float aspect() const { return 720.f / 480.f; /* todo */ }
	float near_clip() const { return 0.05f; }
	float far_clip() const { return 4000.f; }
};

} // namespace blue_sky

#endif // BLUE_SKY_CAMERA_H