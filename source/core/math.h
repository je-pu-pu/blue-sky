#pragma once

#include <core/type.h>

namespace math
{
	using core::Vector3;
	using core::Color;

template<> inline Vector3 chase( Vector3 value, Vector3 target, Vector3 speed )
{
	value.x() = chase( value.x(), target.x(), speed.x() );
	value.y() = chase( value.y(), target.y(), speed.y() );
	value.z() = chase( value.z(), target.z(), speed.z() );

	return value;
}

template<> inline Vector3 chase( Vector3 value, Vector3 target, float_t speed )
{
	value.x() = chase( value.x(), target.x(), speed );
	value.y() = chase( value.y(), target.y(), speed );
	value.z() = chase( value.z(), target.z(), speed );

	return value;
}

template<> inline Color chase( Color value, Color target, Color speed )
{
	value.r() = chase( value.r(), target.r(), speed.r() );
	value.g() = chase( value.g(), target.g(), speed.g() );
	value.b() = chase( value.b(), target.b(), speed.b() );
	value.a() = chase( value.a(), target.a(), speed.a() );

	return value;
}

template<> inline Color chase( Color value, Color target, float_t speed )
{
	value.r() = chase( value.r(), target.r(), speed );
	value.g() = chase( value.g(), target.g(), speed );
	value.b() = chase( value.b(), target.b(), speed );
	value.a() = chase( value.a(), target.a(), speed );

	return value;
}

} // namespace math
