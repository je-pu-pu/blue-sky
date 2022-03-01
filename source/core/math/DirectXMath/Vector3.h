#pragma once

#include <DirectXMath.h>
#include <ostream>
#include <cmath>

namespace direct_x_math
{

class Matrix;

/**
 * �����I�� DirectXMath ���g�p���� Vector3
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

}; // namespace direct_x_math
