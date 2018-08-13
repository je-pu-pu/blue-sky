#pragma once

#include <GameObject/GameObject.h>
#include <blue_sky/type.h>

namespace blue_sky
{

class Player;

/**
 * カメラ
 *
 */
class Camera : public GameObject
{
public:

private:
	Vector position_;					///< 視点
	Vector look_at_;					///< 注視点
	Vector up_;							///< 上

	Vector front_;						///< 前
	Vector right_;						///< 右

	Vector rotate_degree_;				///< XYZ 各軸に対する回転角度
	Vector rotate_degree_target_;		///< XYZ 各軸に対する回転角度 ( 目標 )

	float_t fov_;						///< FOV
	float_t fov_target_;				///< FOV ( 目標 )
	float_t fov_default_;				///< FOV ( 標準 )

	float_t aspect_;					///< アスペクト比

	float_t rotate_chase_speed_;		///< 回転速度

protected:
	float_t get_collision_width() const override { return 0.f; }
	float_t get_collision_height() const override { return 0.f; }
	float_t get_collision_depth() const override { return 0.f; }

public:
	Camera();
	~Camera();

	void restart();

	Vector& position() { return position_; }

	const Vector& position() const { return position_; }
	const Vector& look_at() const { return look_at_; }
	const Vector& up() const { return up_; }

	const Vector& front() const { return front_; }
	const Vector& right() const { return right_; }

	Vector& rotate_degree() { return rotate_degree_; }
	Vector& rotate_degree_target() { return rotate_degree_target_; }

	const Vector& rotate_degree() const { return rotate_degree_; }
	const Vector& rotate_degree_target() const { return rotate_degree_target_; }

	float fov() const { return fov_; }
	void set_fov( float );

	float get_fov_target() const { return fov_target_; }
	void set_fov_target( float );

	float get_fov_default() const { return fov_default_; }
	void set_fov_default( float f ) { fov_ = fov_default_ = f; }

	void reset_fov() { fov_ = fov_target_ = fov_default_; }

	float get_rotate_chase_speed() { return rotate_chase_speed_; }
	void set_rotate_chase_speed( float s ) { rotate_chase_speed_ = s; }
	void reset_rotate_chase_speed() { rotate_chase_speed_ = 0.2f; }

	void update_with_player( const Player* );
	void update();

	void set_aspect( float a ) { aspect_ = a; }
	float aspect() const { return aspect_; }
	float near_clip() const { return 0.05f; }
	float far_clip() const { return 3000.f; }
};

} // namespace blue_sky
