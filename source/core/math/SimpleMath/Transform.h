#pragma once

#include "Quaternion.h"
#include <DirectXTK/SimpleMath.h>
#include <LinearMath/btTransform.h>

namespace core::simple_math
{

class Transform
{
public:
	using Vector3 = DirectX::SimpleMath::Vector3;
	// using Quaternion = DirectX::SimpleMath::Quaternion;

private:
	Vector position_;
	Quaternion rotation_;

public:
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

	Vector right() const { return Vector( Vector3::Transform( Vector3::Right, rotation_ ) ); }
	Vector up() const { return Vector( Vector3::Transform( Vector3::Up, rotation_ ) ); }
	Vector forward() const { return Vector( Vector3::Transform( Vector3::Forward, rotation_ ) ); }

	operator btTransform () const { return btTransform( reinterpret_cast< const btQuaternion& >( rotation_ ), reinterpret_cast< const btVector3& >( position_ ) ); }

	/*
	Transform operator * ( const Transform& t ) const { return Transform( transform_ * t.transform_ ); }
	void operator *= ( const Transform& t ) { transform_ *= t.transform_; }
	*/
};

} // namespace core::simple_math
