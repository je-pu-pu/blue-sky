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
	vector3 default_up_;			///< �f�t�H���g��

	vector3 look_at_under_;			///< �����_ ( ���� )
	vector3 up_under_;				///< �� ( ���� )

	int rotate_step_x_;				///< ��]�p�x�X�e�b�v�� ( X Axis )
	int rotate_step_y_;				///< ��]�p�x�X�e�b�v�� ( Y Axis )

	vector3 rotate_degree_;			///< XYZ �e���ɑ΂����]�p�x
	vector3 rotate_degree_target_;	///< XYZ �e���ɑ΂����]�p�x ( �ڕW )

	float fov_;						///< FOV
	float under_view_rate_;			///< �^�����_�̓x���� ( 0.f .. 1.f )

	int panorama_y_division_;		///< �p�m���}������
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