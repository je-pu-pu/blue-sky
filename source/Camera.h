#pragma once

#include <core/type.h>

namespace blue_sky
{

class Player;

/**
 * �J����
 *
 * @todo GameObject ������ ActiveObjectManager �ŊǗ�����
 */
class alignas( 16 ) Camera // : public GameObject
{
public:

private:
	Vector position_;					///< ���_
	Vector look_at_;					///< �����_
	Vector up_;							///< ��

	Vector default_front_;				///< �f�t�H���g���_
	Vector default_right_;				///< �f�t�H���g��
	Vector default_up_;					///< �f�t�H���g��

	Vector front_;						///< �O
	Vector right_;						///< �E

	Vector rotate_degree_;				///< XYZ �e���ɑ΂����]�p�x
	Vector rotate_degree_target_;		///< XYZ �e���ɑ΂����]�p�x ( �ڕW )

	float_t fov_;						///< FOV
	float_t fov_target_;				///< FOV ( �ڕW )
	float_t fov_default_;				///< FOV ( �W�� )

	float_t aspect_;					///< �A�X�y�N�g��

	float_t rotate_chase_speed_;		///< ��]���x

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

	static void* operator new ( size_t size )
	{
        return _aligned_malloc( size, 16 );
    }

	static void operator delete ( void* p )
	{
		_aligned_free( p );
	}
};

} // namespace blue_sky
