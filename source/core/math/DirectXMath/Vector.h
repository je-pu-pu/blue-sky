#pragma once

#include <DirectXMath.h>
#include <stdlib.h>

namespace direct_x_math
{

class Matrix;

/**
 * “à•”“I‚É DirectXMath ‚ðŽg—p‚µ‚½ Vector
 *
 */
class alignas( 16 ) Vector
{
public:
	typedef float			UnitType;

private:
	DirectX::XMVECTOR value_;

	Vector( const DirectX::XMVECTOR& v )
		: value_( v )
	{

	}

public:
	Vector()
	{

	}

	Vector( UnitType x, UnitType y, UnitType z, UnitType w )
	{
		set( x, y, z, w );
	}

	inline UnitType x() const { UnitType v; DirectX::XMVectorGetXPtr( & v, value_ ); return v; }
	inline UnitType y() const { UnitType v; DirectX::XMVectorGetYPtr( & v, value_ ); return v; }
	inline UnitType z() const { UnitType v; DirectX::XMVectorGetZPtr( & v, value_ ); return v; }
	inline UnitType w() const { UnitType v; DirectX::XMVectorGetWPtr( & v, value_ ); return v; }

	inline Vector3 xyz() const { return Vector3( x(), y(), z() ); }

	inline void set_x( UnitType v ) { value_ = DirectX::XMVectorSetX( value_, v ); }
	inline void set_y( UnitType v ) { value_ = DirectX::XMVectorSetY( value_, v ); }
	inline void set_z( UnitType v ) { value_ = DirectX::XMVectorSetZ( value_, v ); }
	inline void set_w( UnitType v ) { value_ = DirectX::XMVectorSetW( value_, v ); }

	void set( UnitType x, UnitType y, UnitType z, UnitType w )
	{
		value_ = DirectX::XMVectorSet( x, y, z, w );
	}

	UnitType length() const
	{
		return DirectX::XMVectorGetX( DirectX::XMVector3Length( value_ ) );
	}

	Vector& normalize()
	{
		value_ = DirectX::XMVector3Normalize( value_ );

		return *this;
	}

	Vector operator - () const
	{
		return Vector( DirectX::XMVectorNegate( value_ ) );
	}

	Vector& operator *= ( UnitType x )
	{
		value_ = ( *this * x ).value_;

		return *this;
	}

	Vector& operator /= ( UnitType x )
	{
		value_ = ( *this / x ).value_;

		return *this;
	}

	operator DirectX::XMVECTOR () const
	{
		return value_;
	}

	friend Vector operator + ( const Vector& v1, const Vector& v2 )
	{
		return DirectX::XMVectorAdd( v1.value_, v2.value_ );
	}

	friend Vector operator - ( const Vector& v1, const Vector& v2 )
	{
		return DirectX::XMVectorSubtract( v1.value_, v2.value_ );
	}

	friend Vector& operator += ( Vector& v1, const Vector& v2 )
	{
		v1.value_ = DirectX::XMVectorAdd( v1.value_, v2.value_ );
		return v1;
	}

	friend Vector& operator -= ( Vector& v1, const Vector& v2 )
	{
		v1.value_ = DirectX::XMVectorSubtract( v1.value_, v2.value_ );
		return v1;
	}

	friend Vector operator * ( const Vector&, const Matrix& );

	friend Vector operator * ( const Vector& v, UnitType x )
	{
		return DirectX::XMVectorScale( v.value_, x );
	}

	friend Vector operator / ( const Vector& v, UnitType x )
	{
		return DirectX::XMVectorDivide( v.value_, DirectX::XMVectorSet( x, x, x, x ) );
	}

	static void* operator new ( size_t size )
	{
		return _aligned_malloc( size, 16 );
	}

	static void operator delete ( void* p )
	{
		_aligned_free( p );
	}

}; // class Vector

}; // namespace direct_x_math
