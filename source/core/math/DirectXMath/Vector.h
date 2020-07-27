#pragma once

#include "Vector3.h"
#include "Vector2.h"
#include <DirectXMath.h>
// #include <stdlib.h>

#include <ostream>

class btVector3;

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
	using UnitType = float;

private:
	DirectX::XMVECTOR value_;

	explicit Vector( const DirectX::XMVECTOR& v )
		: value_( v )
	{

	}

public:
	Vector()
		: value_()
	{

	}

	Vector( UnitType x, UnitType y, UnitType z, UnitType w = 0.f )
		: value_()
	{
		set( x, y, z, w );
	}

	explicit Vector( const btVector3& v )
		: value_( reinterpret_cast< const DirectX::XMVECTOR& >( v ) )
	{ }

	operator DirectX::XMVECTOR () const	{ return value_; }

	operator btVector3& () { return reinterpret_cast< btVector3& >( value_ ); }
	operator const btVector3& () const { return reinterpret_cast< const btVector3& >( value_ ); }

	inline UnitType x() const { return DirectX::XMVectorGetX( value_ ); }
	inline UnitType y() const { return DirectX::XMVectorGetY( value_ ); }
	inline UnitType z() const { return DirectX::XMVectorGetZ( value_ ); }
	inline UnitType w() const { return DirectX::XMVectorGetW( value_ ); }

	inline Vector2 xy() const { return Vector2( x(), y() ); }
	inline Vector2 yz() const { return Vector2( y(), z() ); }
	inline Vector2 xz() const { return Vector2( x(), z() ); }

	inline Vector3 xyz() const { return Vector3( x(), y(), z() ); }

	inline void set_x( UnitType v ) { value_ = DirectX::XMVectorSetX( value_, v ); }
	inline void set_y( UnitType v ) { value_ = DirectX::XMVectorSetY( value_, v ); }
	inline void set_z( UnitType v ) { value_ = DirectX::XMVectorSetZ( value_, v ); }
	inline void set_w( UnitType v ) { value_ = DirectX::XMVectorSetW( value_, v ); }

	void set( UnitType x, UnitType y, UnitType z, UnitType w = 0.f )
	{
		value_ = DirectX::XMVectorSet( x, y, z, w );
	}

	UnitType length() const { return DirectX::XMVectorGetX( DirectX::XMVector3Length( value_ ) ); }
	Vector& normalize() { value_ = DirectX::XMVector3Normalize( value_ ); return *this; }

	Vector operator - () const { return Vector( DirectX::XMVectorNegate( value_ ) ); }
	bool operator == ( Vector v ) const { return x() == v.x() && y() == v.y() && z() == v.z() && w() == v.w(); }

	friend Vector operator + ( const Vector& v1, const Vector& v2 ) { return Vector( DirectX::XMVectorAdd( v1.value_, v2.value_ ) ); }
	friend Vector operator - ( const Vector& v1, const Vector& v2 ) { return Vector( DirectX::XMVectorSubtract( v1.value_, v2.value_ ) ); }
	friend Vector operator * ( const Vector& v, UnitType x ) { return Vector( DirectX::XMVectorScale( v.value_, x ) ); }
	friend Vector operator / ( const Vector& v, UnitType x ) { return Vector( DirectX::XMVectorScale( v.value_, 1.f / x ) ); }
	
	friend Vector operator * ( const Vector&, const Matrix& );

	friend Vector& operator += ( Vector& v1, const Vector& v2 ) { v1 = v1 + v2; return v1; }
	friend Vector& operator -= ( Vector& v1, const Vector& v2 ) { v1 = v1 - v2; return v1; }
	friend Vector& operator *= ( Vector& v, UnitType x ) { v = v * x; return v; }
	friend Vector& operator /= ( Vector& v, UnitType x ) { v = v / x; return v; }

	static Vector lerp( const Vector& v1, const Vector& v2, UnitType t )
	{
		return v1 + ( ( v2 - v1 ) * t );
	}

	friend std::ostream& operator << ( std::ostream& out, const Vector& v )
	{
	    return out << "( " << v.x() << ", " << v.y() << ", " << v.z() << " )";
	}

	UnitType dot( const Vector& v2 ) const { return DirectX::XMVectorGetX( DirectX::XMVector3Dot( value_, v2.value_ ) ); }
	Vector cross( const Vector& v2 ) const { return Vector( DirectX::XMVector3Cross( value_, v2.value_ ) ); }

	static void* operator new ( size_t size ) { return _aligned_malloc( size, 16 ); }
	static void operator delete ( void* p ) { _aligned_free( p ); }

	static const Vector Zero;

	static const Vector Left;
	static const Vector Right;
	static const Vector Up;
	static const Vector Down;
	static const Vector Forward;
	static const Vector Back;

}; // class Vector

}; // namespace direct_x_math
