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
	vector3 look_at_;		///< �����_
	vector3 up_;			///< ��

	vector3 default_front_;	///< �f�t�H���g���_
	vector3 default_up_;	///< �f�t�H���g��

	vector3 look_at_under_;	///< �����_ ( ���� )
	vector3 up_under_;		///< �� ( ���� )

	float direction_degree_;			///< �p�x ( Y Axis )
	float direction_degree_target_ ;	///< �p�x ( Y Axis ) ( �ڕW )

	float fov_;			///< FOV
	float under_view_rate_;	///< �^�����_�̓x���� ( 0.f .. 1.f )



	int panorama_y_division_;			///< �p�m���}������
public:
	Camera();
	~Camera();

	void set_panorama_y_division( int d ) { panorama_y_division_ = d; }

	vector3& position() { return position_; }
	const vector3& look_at() { return look_at_; }
	const vector3& up() const { return up_; }


	vector3 get_look_at_part( int ) const;
	vector3 get_up_part( int ) const;

	float get_direction_degree() const { return direction_degree_; }
	void set_direction_degree_target( float d ) { direction_degree_target_ = d; }

	const float& fov() const { return fov_; }
	void set_fov( float );

	void update();

	void set_under_view_rate( float );
	float get_under_view_rate() const { return under_view_rate_; }
};

} // namespace blue_sky

#endif // BLUE_SKY_CAMERA_H