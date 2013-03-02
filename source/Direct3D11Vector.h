#ifndef DIRECT_3D_11_VECTOR_H
#define DIRECT_3D_11_VECTOR_H

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

public:
	Direct3D11Vector( UnitType x, UnitType y, UnitType z, UnitType w = 1 )
	{
		value_ = XMVectorSet( x, y, z, w );
	}

	inline UnitType& x() { UnitType* x; XMVectorGetXPtr( x, value_ ); return *x; }
	inline UnitType& y() { UnitType* x; XMVectorGetYPtr( x, value_ ); return *x; }
	inline UnitType& z() { UnitType* x; XMVectorGetZPtr( x, value_ ); return *x; }
	inline UnitType& w() { UnitType* x; XMVectorGetWPtr( x, value_ ); return *x; }

	inline const UnitType& x() const { UnitType* x; XMVectorGetXPtr( x, value_ ); return *x; }
	inline const UnitType& y() const { UnitType* x; XMVectorGetYPtr( x, value_ ); return *x; }
	inline const UnitType& z() const { UnitType* x; XMVectorGetZPtr( x, value_ ); return *x; }
	inline const UnitType& w() const { UnitType* x; XMVectorGetZPtr( x, value_ ); return *x; }

}; // class Direct3D11Vector

#endif // DIRECT_3D_11_VECTOR_H
