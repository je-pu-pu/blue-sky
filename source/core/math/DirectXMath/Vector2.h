#pragma once

#include <DirectXMath.h>
#include <ostream>
#include <cmath>

namespace direct_x_math
{

class Matrix;

/**
 * 内部的に DirectXMath を使用した Vector2
 *
 */
class Vector2
{
public:
	using UnitType = float;

private:
	DirectX::XMFLOAT2 value_;


	Vector2( const DirectX::XMFLOAT2& v )
		: value_( v )
	{

	}

public:
	Vector2()
		: value_()
	{

	}

	Vector2( UnitType x, UnitType y )
		: value_()
	{
		set( x, y );
	}

	inline UnitType& x() { return value_.x; }
	inline UnitType& y() { return value_.y; }

	inline const UnitType& x() const { return value_.x; }
	inline const UnitType& y() const { return value_.y; }

	inline void set( UnitType x, UnitType y )
	{
		value_.x = x;
		value_.y = y;
	}

	inline UnitType length() const { return std::sqrt( x() * x() + y() * y() ); }

	inline bool operator == ( const Vector2& v ) const
	{
		return ( x() == v.x() ) && ( y() == v.y() );
	}

	inline bool operator < ( const Vector2& v ) const
	{
		return
			( ( x() <  v.x() ) ) ||
			( ( x() == v.x() ) && ( y() <  v.y() ) );
	}

	friend std::ostream& operator << ( std::ostream& out, const Vector2& v )
	{
	    return out << "( " << v.x() << ", " << v.y() << " )";
	}

	inline UnitType cross( const Vector2& v2 ) const
	{
		return DirectX::XMVectorGetX( DirectX::XMVector2Cross( DirectX::XMLoadFloat2( & value_ ), DirectX::XMLoadFloat2( & v2.value_ ) ) );
	}

}; // class Vector2

}; // namespace direct_x_math
