#ifndef BLUE_SKY_MATH_H
#define BLUE_SKY_MATH_H

class vector3;
class Direct3D11Color;

namespace math
{

template<> inline vector3 chase( vector3 value, vector3 target, vector3 speed )
{
	value.x() = chase( value.x(), target.x(), speed.x() );
	value.y() = chase( value.y(), target.y(), speed.y() );
	value.z() = chase( value.z(), target.z(), speed.z() );

	return value;
}

template<> inline vector3 chase( vector3 value, vector3 target, float_t speed )
{
	value.x() = chase( value.x(), target.x(), speed );
	value.y() = chase( value.y(), target.y(), speed );
	value.z() = chase( value.z(), target.z(), speed );

	return value;
}

template<> inline Direct3D11Color chase( Direct3D11Color value, Direct3D11Color target, Direct3D11Color speed )
{
	value.r() = chase( value.r(), target.r(), speed.r() );
	value.g() = chase( value.g(), target.g(), speed.g() );
	value.b() = chase( value.b(), target.b(), speed.b() );
	value.a() = chase( value.a(), target.a(), speed.a() );

	return value;
}

template<> inline Direct3D11Color chase( Direct3D11Color value, Direct3D11Color target, float_t speed )
{
	value.r() = chase( value.r(), target.r(), speed );
	value.g() = chase( value.g(), target.g(), speed );
	value.b() = chase( value.b(), target.b(), speed );
	value.a() = chase( value.a(), target.a(), speed );

	return value;
}

}

#endif // BLUE_SKY_MATH_H