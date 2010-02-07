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

	Color( RGBQUAD color )
		: color_( color )
	{ }

	~Color() { }

	operator const RGBQUAD& () const { return color_; }
};

}

#endif // ART_MODEL_H