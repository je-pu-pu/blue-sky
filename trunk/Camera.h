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
	vector3 position_;				///< ���_
	vector3 look_at_;				///< �����_
	vector3 up_;					///< ��

	vector3 default_front_;			///< �f�t�H���g���_
	vector3 default_right_;			///< �f�t�H���g��
	vector3 default_up_;			///< �f�t�H���g��

	vector3 front_;					///< �O
	vector3 right_;					///< �E

	vector3 rotate_degree_;			///< XYZ �e���ɑ΂����]�p�x
	vector3 rotate_degree_target_;	///< XYZ �e���ɑ΂����]�p�x ( �ڕW )

	float fov_;						///< FOV
	float fov_target_;				///< FOV ( �ڕW )
	float fov_default_;				///< FOV ( �W�� )

	float aspect_;					///< �A�X�y�N�g��

	float rotate_chase_speed_;		///< ��]���x

public:
	Camera();
	~Camera();

	vector3& position() { return position_; }
	const vector3& look_at() { return look_at_; }
	const vector3& up() const { return up_; }

	const vector3& front() const { return front_; }
	const vector3& right() const { return right_; }

	vector3& rotate_degree() { return rotate_degree_; }
	vector3& rotate_degree_target() { return rotate_degree_target_; }

	const vector3& rotate_degree() const { return rotate_degree_; }
	const vector3& rotate_degree_target() const { return rotate_degree_target_; }

	float fov() const { return fov_; }
	void set_fov( float );

	float get_fov_target() const { return fov_target_; }
	void set_fov_target( float );

	float get_fov_default() const { return fov_default_; }
	void set_fov_default( float f ) { fov_default_ = f; }

	void reset_fov() { fov_ = fov_target_ = fov_default_; }

	float get_rotate_chase_speed() { return rotate_chase_speed_; }
	void set_rotate_chase_speed( float s ) { rotate_chase_speed_; }
	void reset_rotate_chase_speed() { rotate_chase_speed_ = 0.2f; }

	void update();

	void set_aspect( float a ) { aspect_ = a; }
	float aspect() const { return aspect_; }
	float near_clip() const { return 0.05f; }
	float far_clip() const { return 4000.f; }
};

} // namespace blue_sky

#endif // BLUE_SKY_CAMERA_H