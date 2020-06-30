#pragma once

#include "../type.h"
#include <optional>

namespace core
{

/**
 * ü•ª ( 2D )
 */
class LineSegment2
{
private:
	Vector2 a_;
	Vector2 b_;

public:
	LineSegment2() = delete;
	LineSegment2( const Vector2& a, const Vector2& b )
		: a_( a )
		, b_( b )
	{
	}

	/**
	 * ü•ª“¯m‚ÌŒğ·”»’è
	 */
	std::optional< Vector2 > intersection( const LineSegment2& ls ) const
	{
		const auto d = ( b_.x() - a_.x() ) * ( ls.b_.y() - ls.a_.y() ) - ( b_.y() - a_.y() ) * ( ls.b_.x() - ls.a_.x() );

		if ( d == 0.f )
		{
			return std::nullopt;
		}

		const auto u = ( ( ls.a_.x() - a_.x() ) * ( ls.b_.y() - ls.a_.y() ) - ( ls.a_.y() - a_.y() ) * ( ls.b_.x() - ls.a_.x() ) ) / d;
		const auto v = ( ( ls.a_.x() - a_.x() ) * ( b_.y() - a_.y() ) - ( ls.a_.y() - a_.y() ) * ( b_.x() - a_.x() ) ) / d;

		if ( u < 0.f || u > 1.f || v < 0.f || v > 1.f )
		{
			return std::nullopt;
		}

		return std::make_optional< Vector2 >( a_.x() + u * ( b_.x() - a_.x() ), a_.y() + u * ( b_.y() - a_.y() ) );
	}

	static std::optional< Vector2 > intersection( const LineSegment2& a, const LineSegment2& b )
	{
		LineSegment2 ls( a );
		return a.intersection( b );
	}
};

}; // namespace core
