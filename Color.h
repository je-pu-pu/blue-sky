#ifndef ART_COLOR_H
#define ART_COLOR_H

#include "Type.h"
#include <windows.h>

namespace art
{

/**
 * 
 */
class Color
{
public:
	typedef unsigned char UnitType;
private:
	RGBQUAD color_;

public:
	Color( UnitType r = 0, UnitType g = 0, UnitType b = 0, UnitType a = 255 )
	{
		color_.rgbRed = r;
		color_.rgbGreen = g;
		color_.rgbBlue = b;
		color_.rgbReserved = a;
	}
	
	Color( COLORREF color )
	{
		color_.rgbRed   = GetRValue( color );
		color_.rgbGreen = GetGValue( color );
		color_.rgbBlue  = GetBValue( color );
		color_.rgbReserved = 0xFF;
	}

	Color( RGBQUAD color )
		: color_( color )
	{ }

	~Color() { }

	operator RGBQUAD& () { return color_; }
	operator const RGBQUAD& () const { return color_; }

	UnitType r() const { return color_.rgbRed; }
	UnitType g() const { return color_.rgbGreen; }
	UnitType b() const { return color_.rgbBlue; }
};

}

#endif // ART_MODEL_H