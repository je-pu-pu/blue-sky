#pragma once

#include <core/math/Vector.h>
#include <LinearMath/btTransform.h>

namespace core::math::bullet
{

class Quaternion
{
public:
	using Vector = direct_x_math::Vector;

private:
	btQuaternion value_;

public:
	Quaternion() = default;

	explicit Quaternion( const btQuaternion& q )
		: value_( q )
	{ }

	Quaternion( const btScalar& yaw, const btScalar& pitch, const btScalar& roll )
		: value_( yaw, pitch, roll )
	{ }

	void get_yaw_pitch_roll( btScalar& yaw, btScalar& pitch, btScalar& roll ) const
	{
		value_.getEulerZYX( yaw, pitch, roll );
	}

	void set_yaw_pitch_roll( const btScalar& yaw, const btScalar& pitch, const btScalar& roll )
	{
		value_.setEuler( yaw, pitch, roll );
	}

	Quaternion operator * ( const Quaternion& q ) { return Quaternion( value_ * q.value_ ); }

	operator btQuaternion& () { return value_; }
	operator const btQuaternion& () const { return value_; }

	operator Vector& () { return reinterpret_cast< Vector& >( value_ ); }
	operator const Vector& () const { return reinterpret_cast< const Vector& >( value_ ); }
};

} // namespace core::math::bullet
