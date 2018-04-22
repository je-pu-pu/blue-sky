#ifndef BLUE_SKY_CAMERA_H
#define BLUE_SKY_CAMERA_H

#include <core/Vector3.h>
// #include <GameObject/GameObject.h>

namespace blue_sky
{

class Player;

/**
 * �J����
 *
 * @todo GameObject ������ ActiveObjectManager �ŊǗ�����
 */
class Camera // : public GameObject
{
public:

private:
	Vector3 position_;					///< ���_
	Vector3 look_at_;					///< �����_
	Vector3 up_;						///< ��

	Vector3 default_front_;				///< �f�t�H���g���_
	Vector3 default_right_;				///< �f�t�H���g��
	Vector3 default_up_;				///< �f�t�H���g��

	Vector3 front_;						///< �O
	Vector3 right_;						///< �E

	Vector3 rotate_degree_;				///< XYZ �e���ɑ΂����]�p�x
	Vector3 rotate_degree_target_;		///< XYZ �e���ɑ΂����]�p�x ( �ڕW )

	float_t fov_;						///< FOV
	float_t fov_target_;				///< FOV ( �ڕW )
	float_t fov_default_;				///< FOV ( �W�� )

	float_t aspect_;					///< �A�X�y�N�g��

	float_t rotate_chase_speed_;		///< ��]���x

public:
	Camera();
	~Camera();

	void restart();

	Vector3& position() { return position_; }

	const Vector3& position() const { return position_; }
	const Vector3& look_at() const { return look_at_; }
	const Vector3& up() const { return up_; }

	const Vector3& front() const { return front_; }
	const Vector3& right() const { return right_; }

	Vector3& rotate_degree() { return rotate_degree_; }
	Vector3& rotate_degree_target() { return rotate_degree_target_; }

	const Vector3& rotate_degree() const { return rotate_degree_; }
	const Vector3& rotate_degree_target() const { return rotate_degree_target_; }

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

#endif // BLUE_SKY_CAMERA_H