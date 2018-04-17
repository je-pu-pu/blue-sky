#ifndef DIRECT_3D_11_VECTOR_H
#define DIRECT_3D_11_VECTOR_H

#include "Direct3D11Common.h"
#include <stdlib.h>

class Direct3D11Matrix;

/**
 * Direct3D 11 Vector
 *
 */
class Direct3D11Vector
{
public:
	typedef float			UnitType;
	typedef int				IntType;
	typedef unsigned int	UintType;

private:
	alignas( 16 ) DirectX::XMVECTOR value_;

	Direct3D11Vector( const DirectX::XMVECTOR& v )
		: value_( v )
	{

	}

public:
	Direct3D11Vector()
	{

	}

	Direct3D11Vector( UnitType x, UnitType y, UnitType z, UnitType w = 1 )
	{
		value_ = DirectX::XMVectorSet( x, y, z, w );
	}

	static Direct3D11Vector FromXMVECTOR( const DirectX::XMVECTOR& v )
	{
		return Direct3D11Vector( v );
	}

	inline UnitType x() const { UnitType x; DirectX::XMVectorGetXPtr( & x, value_ ); return x; }
	inline UnitType y() const { UnitType x; DirectX::XMVectorGetYPtr( & x, value_ ); return x; }
	inline UnitType z() const { UnitType x; DirectX::XMVectorGetZPtr( & x, value_ ); return x; }
	inline UnitType w() const { UnitType x; DirectX::XMVectorGetWPtr( & x, value_ ); return x; }

	UnitType length() const
	{
		return DirectX::XMVectorGetX( DirectX::XMVector3Length( value_ ) );
	}

	Direct3D11Vector& normalize()
	{
		value_ = DirectX::XMVector3Normalize( value_ );

		return *this;
	}

	Direct3D11Vector operator - () const
	{
		return Direct3D11Vector( DirectX::XMVectorNegate( value_ ) );
	}

	Direct3D11Vector& operator *= ( UnitType x )
	{
		value_ = ( *this * x ).value_;

		return *this;
	}

	Direct3D11Vector& operator /= ( UnitType x )
	{
		value_ = ( *this / x ).value_;

		return *this;
	}

	operator DirectX::XMVECTOR () const
	{
		return value_;
	}

	friend Direct3D11Vector operator + ( const Direct3D11Vector& v1, const Direct3D11Vector& v2 )
	{
		return DirectX::XMVectorAdd( v1.value_, v2.value_ );
	}

	friend Direct3D11Vector operator - ( const Direct3D11Vector& v1, const Direct3D11Vector& v2 )
	{
		return DirectX::XMVectorSubtract( v1.value_, v2.value_ );
	}

	friend Direct3D11Vector& operator += ( Direct3D11Vector& v1, const Direct3D11Vector& v2 )
	{
		v1.value_ = DirectX::XMVectorAdd( v1.value_, v2.value_ );
		return v1;
	}

	friend Direct3D11Vector& operator -= ( Direct3D11Vector& v1, const Direct3D11Vector& v2 )
	{
		v1.value_ = DirectX::XMVectorSubtract( v1.value_, v2.value_ );
		return v1;
	}

	friend Direct3D11Vector operator * ( const Direct3D11Vector&, const Direct3D11Matrix& );

	friend Direct3D11Vector operator * ( const Direct3D11Vector& v, UnitType x )
	{
		return DirectX::XMVectorScale( v.value_, x );
	}

	friend Direct3D11Vector operator / ( const Direct3D11Vector& v, UnitType x )
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

}; // class Direct3D11Vector

#endif // DIRECT_3D_11_VECTOR_H
