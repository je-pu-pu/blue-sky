#pragma once

#include <core/math/Vector.h>
#include <LinearMath/btTransform.h>

namespace core::bullet
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

	Vector get_euler_zyx() const
	{
		float_t x, y, z;

		value_.getEulerZYX( z, y, x );

		return Vector( x, y, z );
	}

	void set_euler_zyx( const btScalar& yaw_z, const btScalar& pitch_y, const btScalar& roll_x )
	{
		value_.setEulerZYX( yaw_z, pitch_y, roll_x );
	}

	Quaternion operator * ( const Quaternion& q ) { return Quaternion( value_ * q.value_ ); }

	operator btQuaternion& () { return value_; }
	operator const btQuaternion& () const { return value_; }

	operator Vector& () { return reinterpret_cast< Vector& >( value_ ); }
	operator const Vector& () const { return reinterpret_cast< const Vector& >( value_ ); }
};

} // namespace core::bullet
