#pragma once

#include <core/type.h>

namespace core::ui
{

/**
 * 仮想座標系（1920x1080 基準）の矩形
 */
struct Rect
{
	float_t x = 0.f;
	float_t y = 0.f;
	float_t w = 0.f;
	float_t h = 0.f;

	Rect() = default;
	Rect( float_t x_, float_t y_, float_t w_, float_t h_ ) : x( x_ ), y( y_ ), w( w_ ), h( h_ ) { }

	float_t left() const { return x; }
	float_t top() const { return y; }
	float_t right() const { return x + w; }
	float_t bottom() const { return y + h; }
	float_t center_x() const { return x + w * 0.5f; }
	float_t center_y() const { return y + h * 0.5f; }

	/// 点が矩形内にあるか（右端・下端は含まない）
	bool contains( float_t px, float_t py ) const
	{
		return px >= x && px < x + w && py >= y && py < y + h;
	}

	/// 2 つの矩形が重なるか
	bool intersects( const Rect& r ) const
	{
		return x < r.right() && r.x < right() && y < r.bottom() && r.y < bottom();
	}

	/// 2 つの矩形の共通部分（重ならなければ幅・高さ 0）
	Rect intersection( const Rect& r ) const
	{
		const float_t l = x > r.x ? x : r.x;
		const float_t t = y > r.y ? y : r.y;
		const float_t rr = right() < r.right() ? right() : r.right();
		const float_t b = bottom() < r.bottom() ? bottom() : r.bottom();

		if ( rr <= l || b <= t )
		{
			return Rect( l, t, 0.f, 0.f );
		}

		return Rect( l, t, rr - l, b - t );
	}

	/// 四辺を内側に縮めた矩形
	Rect inset( float_t d ) const
	{
		return inset( d, d );
	}

	Rect inset( float_t dx, float_t dy ) const
	{
		const float_t nw = w - dx * 2.f;
		const float_t nh = h - dy * 2.f;
		return Rect( x + dx, y + dy, nw > 0.f ? nw : 0.f, nh > 0.f ? nh : 0.f );
	}

	bool operator == ( const Rect& r ) const { return x == r.x && y == r.y && w == r.w && h == r.h; }
	bool operator != ( const Rect& r ) const { return ! ( *this == r ); }

}; // struct Rect

} // namespace core::ui
