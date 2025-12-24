#pragma once

#include "Quaternion.h"
#include "Vector.h"
#include <LinearMath/btTransform.h>

namespace core::math::direct_x_math
{

class Transform
{
private:
	Vector position_;
	Quaternion rotation_;

public:
	Transform() = default;
	~Transform() = default;

	void set_identity()
	{
		position_.set( 0.f, 0.f, 0.f );
		rotation_.set_identity();
	}

	Vector& get_position() { return position_; }
	const Vector& get_position() const { return position_; }
	void set_position( const Vector& p ) { position_ = p; }

	Quaternion& get_rotation() { return rotation_; }
	const Quaternion& get_rotation() const { return rotation_; }
	void set_rotation( const Quaternion& q ) { rotation_ = q; }

	Vector right() const { return Vector::transform( Vector::Right, rotation_ ); }
	Vector up() const { return Vector::transform( Vector::Up, rotation_ ); }
	Vector forward() const { return Vector::transform( Vector::Forward, rotation_ ); }

	operator btTransform () const { return btTransform( reinterpret_cast< const btQuaternion& >( rotation_ ), reinterpret_cast< const btVector3& >( position_ ) ); }

	/*
	Transform operator * ( const Transform& t ) const { return Transform( transform_ * t.transform_ ); }
	void operator *= ( const Transform& t ) { transform_ *= t.transform_; }
	*/

	static Transform identity()
	{
		Transform t;
		t.set_identity();
		return t;
	}
};

} // namespace core::math::direct_x_math
