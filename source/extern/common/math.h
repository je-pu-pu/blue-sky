// *********************************************************
// math.h
// copyright (c) JE all rights reserved
// 2010/05/31
// *********************************************************

#ifndef COMMON_MATH_H
#define COMMON_MATH_H

#define _USE_MATH_DEFINES

#include <cmath>
#include <algorithm>

namespace math
{

const float pi = 3.14159265358979323846f;

template< typename T > inline T clamp( T value, T min_value, T max_value )
{
	value = std::max( min_value, value );
	value = std::min( max_value, value );

	return value;
}

template< typename T > inline T degree_to_radian( T degree )
{
	return degree * ( pi / 180 );
}

template< typename T > inline T radian_to_degree( T radian )
{
	return radian * ( 180 / pi );
}

template< typename T, typename ST > inline T chase( T value, T target, ST speed )
{
	if ( value < target )
	{
		value += speed;

		if ( value > target )
		{
			value = target;
		}
	}
	else if ( value > target )
	{	
		value -= speed;

		if ( value < target )
		{
			value = target;
		}
	}

	return value;
}

/**
 * üŒ`•âŠÔ
 */
template< typename T > inline T lerp( T a, T b, float t )
{
	return a + ( t * ( b - a ) );
}

} // namespace math

#endif // COMMON_MATH_H
