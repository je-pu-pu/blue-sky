#pragma once

#include <core/math/Vector.h>
#include <DirectXTK/SimpleMath.h>

namespace core::simple_math
{

class Quaternion
{
private:
	using ValueType = DirectX::SimpleMath::Quaternion;
	using Vector3 = DirectX::SimpleMath::Vector3;

	ValueType value_;

public:
	Quaternion() = default;

	explicit Quaternion( const Quaternion& q )
		: value_( q.value_ )
	{ }

	Quaternion( const float yaw, const float pitch, const float roll )
		: value_( ValueType::CreateFromYawPitchRoll( yaw, pitch, roll ) )
	{ }

	void get_yaw_pitch_roll( float& yaw, float& pitch, float& roll ) const
	{
		Vector3 v = value_.ToEuler();

		yaw = v.y;
		pitch = v.x;
		roll = v.z;
	}

	void set_yaw_pitch_roll( const float& yaw, const float& pitch, const float& roll )
	{
		value_ = ValueType::CreateFromYawPitchRoll( yaw, pitch, roll );
	}

	void set_identity()
	{
		value_ = ValueType::Identity;
	}

	Quaternion operator * ( const Quaternion& q ) const
	{
		Quaternion result;
		result.value_ = value_ * q;
		return Quaternion( result );
	}

	operator const Vector () const { return Vector( value_ ); }
	operator const ValueType& () const { return value_; }
};

} // namespace core::simple_math
