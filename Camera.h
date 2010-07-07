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

	vector3 look_at_under_;			///< 注視点 ( 足元 )
	vector3 up_under_;				///< 上 ( 足元 )

	int rotate_step_x_;				///< 回転角度ステップ数 ( X Axis )
	int rotate_step_y_;				///< 回転角度ステップ数 ( Y Axis )

	vector3 rotate_degree_;			///< XYZ 各軸に対する回転角度
	vector3 rotate_degree_target_;	///< XYZ 各軸に対する回転角度 ( 目標 )

	float fov_;						///< FOV
	float under_view_rate_;			///< 真下視点の度合い ( 0.f .. 1.f )

	int panorama_y_division_;		///< パノラマ分割数
public:
	Camera();
	~Camera();

	void set_panorama_y_division( int d ) { panorama_y_division_ = d; }

	vector3& position() { return position_; }
	const vector3& look_at() { return look_at_; }
	const vector3& up() const { return up_; }

	vector3 get_look_at_part( int ) const;
	vector3 get_up_part( int ) const;

	vector3& rotate_degree() { return rotate_degree_; }
	vector3& rotate_degree_target() { return rotate_degree_target_; }

	const vector3& rotate_degree() const { return rotate_degree_; }
	const vector3& rotate_degree_target() const { return rotate_degree_target_; }

	const float& fov() const { return fov_; }
	void set_fov( float );

	void update();

	void set_under_view_rate( float );
	float get_under_view_rate() const { return under_view_rate_; }

	float aspect() const { return 720.f / ( 480.f / panorama_y_division_ ); }
	float near_clip() const { return 0.05f; }
	float far_clip() const { return 500.f; }

	void step_rotate_x( int );
	void step_rotate_y( int );
};

} // namespace blue_sky

#endif // BLUE_SKY_CAMERA_H