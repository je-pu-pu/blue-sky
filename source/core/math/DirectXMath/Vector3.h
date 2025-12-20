#pragma once

#include <DirectXMath.h>
#include <ostream>
#include <cmath>

namespace core::math::direct_x_math
{

class Matrix;

/**
 * 内部的に DirectXMath を使用した Vector3
 *
 */
class Vector3
{
public:
	using UnitType = float;

private:
	DirectX::XMFLOAT3 value_;


	Vector3( const DirectX::XMFLOAT3& v )
		: value_( v )
	{

	}

public:
	Vector3()
		: value_()
	{

	}

	Vector3( UnitType x, UnitType y, UnitType z )
		: value_( x, y, z )
	{

	}

	inline UnitType& x() { return value_.x; }
	inline UnitType& y() { return value_.y; }
	inline UnitType& z() { return value_.z; }

	inline const UnitType& x() const { return value_.x; }
	inline const UnitType& y() const { return value_.y; }
	inline const UnitType& z() const { return value_.z; }

	inline void set( UnitType x, UnitType y, UnitType z )
	{
		value_.x = x;
		value_.y = y;
		value_.z = z;
	}

	inline UnitType length() const { return std::sqrt( x() * x() + y() * y() + z() * z() ); }

	friend Vector3 operator + ( const Vector3& v1, const Vector3& v2 ) { return Vector3( v1.x() + v2.x(), v1.y() + v2.y(), v1.z() + v2.z() ); }
	friend Vector3 operator - ( const Vector3& v1, const Vector3& v2 ) { return Vector3( v1.x() - v2.x(), v1.y() - v2.y(), v1.z() - v2.z() ); }
	friend Vector3 operator * ( const Vector3& v, UnitType x ) { return Vector3( v.x() * x, v.y() * x, v.z() * x ); }
	friend Vector3 operator / ( const Vector3& v, UnitType x ) { return Vector3( v.x() / x, v.y() / x, v.z() / x ); }

	friend Vector3& operator += ( Vector3& v1, const Vector3& v2 ) { v1 = v1 + v2; return v1; }
	friend Vector3& operator -= ( Vector3& v1, const Vector3& v2 ) { v1 = v1 - v2; return v1; }
	friend Vector3& operator *= ( Vector3& v, UnitType x ) { v = v * x; return v; }
	friend Vector3& operator /= ( Vector3& v, UnitType x ) { v = v / x; return v; }

	inline bool operator == ( const Vector3& v ) const
	{
		return ( x() == v.x() ) && ( y() == v.y() ) && ( z() == v.z() );
	}

	inline bool operator < ( const Vector3& v ) const
	{
		return
			( ( x() <  v.x() ) ) ||
			( ( x() == v.x() ) && ( y() <  v.y() ) ) ||
			( ( x() == v.x() ) && ( y() == v.y() ) && ( z() <  v.z() ) );
	}

	friend std::ostream& operator << ( std::ostream& out, const Vector3& v )
	{
	    return out << "( " << v.x() << ", " << v.y() << ", " << v.z() << " )";
	}

}; // class Vector3

}; // namespace core::math::direct_x_math
