#ifndef DIRECT_3D_11_COLOR_H
#define DIRECT_3D_11_COLOR_H

#include "Direct3D11Common.h"

/**
 * Direct3D 11 Color
 *
 */
class Direct3D11Color : public DirectX::XMFLOAT4
{
public:
	typedef float			UnitType;
	typedef int				IntType;
	typedef unsigned int	UintType;

	static Direct3D11Color White;
	static Direct3D11Color Black;

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

	static Direct3D11Color from_hex( UintType hex )
	{
		return from_256(
			( hex >> 24 ) & 0xFF,
			( hex >> 16 ) & 0xFF,
			( hex >>  8 ) & 0xFF,
			( hex >>  0 ) & 0xFF
		);
	}

	static Direct3D11Color from_hsv( UnitType h, UnitType s, UnitType v )
	{
		float r = v;
		float g = v;
		float b = v;

		if ( s > 0.0f )
		{
			h *= 6.0f;

			const int i = ( int ) h;
			const float f = h - ( float ) i;

			switch ( i )
			{
			default:
			case 0:
				g *= 1 - s * ( 1 - f );
				b *= 1 - s;
				break;
			case 1:
				r *= 1 - s * f;
				b *= 1 - s;
				break;
			case 2:
				r *= 1 - s;
				b *= 1 - s * ( 1 - f );
				break;
			case 3:
				r *= 1 - s;
				g *= 1 - s * f;
				break;
			case 4:
				r *= 1 - s * ( 1 - f );
				g *= 1 - s;
				break;
			case 5:
				g *= 1 - s;
				b *= 1 - s * f;
				break;
			}
		}

		return Direct3D11Color( r, g, b, 1.f );
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
