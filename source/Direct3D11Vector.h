#ifndef DIRECT_3D_11_VECTOR_H
#define DIRECT_3D_11_VECTOR_H

#include "Direct3D11Matrix.h"

#include <d3d11.h>
#include <xnamath.h>

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
	__declspec( align( 16) ) XMVECTOR value_;

	Direct3D11Vector( XMVECTOR v )
		: value_( v )
	{

	}

public:
	Direct3D11Vector( UnitType x, UnitType y, UnitType z, UnitType w = 1 )
	{
		value_ = XMVectorSet( x, y, z, w );
	}

	inline UnitType x() const { UnitType x; XMVectorGetXPtr( & x, value_ ); return x; }
	inline UnitType y() const { UnitType x; XMVectorGetYPtr( & x, value_ ); return x; }
	inline UnitType z() const { UnitType x; XMVectorGetZPtr( & x, value_ ); return x; }
	inline UnitType w() const { UnitType x; XMVectorGetWPtr( & x, value_ ); return x; }

	friend Direct3D11Vector operator * ( const Direct3D11Vector& v, const Direct3D11Matrix& m )
	{
		return XMVector4Transform( v.value_, XMLoadFloat4x4( & m.value_ ) );
	}

}; // class Direct3D11Vector

#endif // DIRECT_3D_11_VECTOR_H
