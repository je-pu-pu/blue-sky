#pragma once

#include <DirectXMath.h>
#include <ostream>

class btQuaternion;

namespace core::math::direct_x_math
{

class Matrix;

class alignas( 16 ) Quaternion
{
public:
	friend class Vector;
	friend class Matrix;

private:
	DirectX::XMVECTOR value_;

	explicit Quaternion( const DirectX::XMVECTOR& q )
		: value_( q )
	{ }

public:
	Quaternion()
		: value_()
	{ }

	explicit Quaternion( float x, float y, float z, float w )
		: value_( DirectX::XMVectorSet( x, y, z, w ) )
	{ }

	explicit Quaternion( const Quaternion& q )
		: value_( q.value_ )
	{ }

	static Quaternion from_yaw_pitch_roll( const float yaw, const float pitch, const float roll )
	{
		return Quaternion( DirectX::XMQuaternionRotationRollPitchYaw( pitch, yaw, roll ) );
	}

	void get_yaw_pitch_roll( float& yaw, float& pitch, float& roll ) const;

	void set_yaw_pitch_roll( const float& yaw, const float& pitch, const float& roll )
	{
		value_ = DirectX::XMQuaternionRotationRollPitchYaw( pitch, yaw, roll );
	}

	void set_identity()
	{
		value_ = DirectX::XMVectorSet( 0.f, 0.f, 0.f, 1.f );
	}

	Quaternion operator * ( const Quaternion& q ) const
	{
		return Quaternion( DirectX::XMQuaternionMultiply( value_, q.value_ ) );
	}

	// operator const Vector () const { return Vector( value_ ); }
	// operator const ValueType& () const { return value_; }

	operator btQuaternion& () { return reinterpret_cast< btQuaternion& >( value_ ); }
	operator const btQuaternion& () const { return reinterpret_cast< const btQuaternion& >( value_ ); }

	static const Quaternion Identity;

	friend std::ostream& operator << ( std::ostream& out, const Quaternion& v );
};

} // namespace core::math::direct_x_math
