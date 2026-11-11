#pragma once

#include <core/ui/Rect.h>

#include <cmath>

namespace core::ui
{

/// 物理ピクセルの矩形（右・下は含まない）
struct PixelRect
{
	long left = 0;
	long top = 0;
	long right = 0;
	long bottom = 0;
};

/**
 * 仮想座標の矩形を物理ピクセルに写す
 *
 * - 各辺を「辺の座標」から 1 つの式（offset + 座標 × scale）で写し、四捨五入する。
 *   隣り合う矩形は共有する辺が同じピクセルになるので、隙間も重なりもできない。
 * - 幅・高さが正なのに 0 ピクセルに丸まったときは 1 ピクセルにする（細い線が消えないように）。
 *
 * 具象 Renderer（blue_sky::ui::Renderer）とテスト用の Mock が同じ規則を使うよう、ここに 1 つだけ置く。
 */
inline PixelRect to_pixel_rect( const Rect& r, float_t scale, float_t offset_x, float_t offset_y )
{
	PixelRect p;
	p.left = std::lround( offset_x + r.x * scale );
	p.top = std::lround( offset_y + r.y * scale );
	p.right = std::lround( offset_x + ( r.x + r.w ) * scale );
	p.bottom = std::lround( offset_y + ( r.y + r.h ) * scale );

	if ( r.w > 0.f && p.right <= p.left )
	{
		p.right = p.left + 1;
	}
	if ( r.h > 0.f && p.bottom <= p.top )
	{
		p.bottom = p.top + 1;
	}

	return p;
}

/// 矩形の四辺を物理ピクセルの境界にそろえた矩形（仮想座標）
inline Rect snap_rect_to_pixels( const Rect& r, float_t scale, float_t offset_x, float_t offset_y )
{
	if ( scale <= 0.f )
	{
		return r;
	}

	const PixelRect p = to_pixel_rect( r, scale, offset_x, offset_y );

	return Rect(
		( static_cast< float_t >( p.left ) - offset_x ) / scale,
		( static_cast< float_t >( p.top ) - offset_y ) / scale,
		static_cast< float_t >( p.right - p.left ) / scale,
		static_cast< float_t >( p.bottom - p.top ) / scale );
}

} // namespace core::ui
