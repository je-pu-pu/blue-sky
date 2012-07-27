#ifndef DIRECT_3D_11_COLOR_H
#define DIRECT_3D_11_COLOR_H

#include <d3d11.h>
#include <xnamath.h>

/**
 * Direct3D 11 Color
 *
 */
class Direct3D11Color : public XMFLOAT4
{
public:
	typedef float	UnitType;
	typedef int		IntType;

public:
	Direct3D11Color( UnitType r, UnitType g, UnitType b, UnitType a )
		: XMFLOAT4( r, g, b, a )
	{

	}

	static Direct3D11Color from_256( IntType r, IntType g, IntType b, IntType a = 255 )
	{
		return Direct3D11Color(
			static_cast< UnitType >( r ) / 255.f,
			static_cast< UnitType >( g ) / 255.f,
			static_cast< UnitType >( b ) / 255.f,
			static_cast< UnitType >( a ) / 255.f
		);
	}

	inline UnitType& r() { return x; }
	inline UnitType& g() { return y; }
	inline UnitType& b() { return z; }
	inline UnitType& a() { return w; }

	inline const UnitType& r() const { return x; }
	inline const UnitType& g() const { return y; }
	inline const UnitType& b() const { return z; }
	inline const UnitType& a() const { return w; }

	// operator XMFLOAT4& () { return *this; }
	// operator const XMFLOAT4& () const { return *this; }

}; // class Direct3D11Color

#endif // DIRECT_3D_11_COLOR_H
