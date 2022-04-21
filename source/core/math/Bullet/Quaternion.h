#pragma once

#include <core/math/Vector.h>
#include <LinearMath/btTransform.h>

namespace core::bullet
{

class Quaternion
{
public:

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

	void set_euler_zyx( const btScalar& yaw_z, const btScalar& pitch_y, const btScalar& roll_x )
	{
		value_.setEulerZYX( yaw_z, pitch_y, roll_x );
	}

	Quaternion operator * ( const Quaternion& q ) { return Quaternion( value_ * q.value_ ); }

	operator btQuaternion& () { return value_; }
	operator const btQuaternion& () const { return value_; }

	operator direct_x_math::Vector& () { return reinterpret_cast< direct_x_math::Vector& >( value_ ); }
	operator const direct_x_math::Vector& () const { return reinterpret_cast< const direct_x_math::Vector& >( value_ ); }
};

} // namespace core::bullet
