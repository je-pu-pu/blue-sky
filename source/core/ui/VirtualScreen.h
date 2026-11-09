#pragma once

#include <core/type.h>

#include <algorithm>

namespace core::ui
{

/**
 * 仮想画面（UI の座標系）と物理画面の対応
 *
 * 短い辺（16:9 より横長なら高さ、縦長なら幅）を設計の大きさに合わせ、長い辺は画面に合わせて伸ばす。
 * 以前は 1920x1080 で固定し、余りを左右・上下の余白にしていたため、21:9 の画面で HUD を端に寄せられなかった
 * （設計上の問題 B13）。16:9 の画面では以前と同じ（1920x1080）。
 */
struct VirtualScreen
{
	float_t width = 0.f;		///< 仮想画面の幅
	float_t height = 0.f;		///< 仮想画面の高さ
	float_t scale = 1.f;		///< 仮想座標 1 の物理ピクセル数

	float_t to_physical_x( float_t x ) const { return x * scale; }
	float_t to_physical_y( float_t y ) const { return y * scale; }
	float_t to_virtual_x( float_t px ) const { return px / scale; }
	float_t to_virtual_y( float_t py ) const { return py / scale; }
};

/// 物理画面の大きさと設計の大きさから仮想画面を求める（物理画面が 0 なら設計の大きさのまま）
inline VirtualScreen compute_virtual_screen( float_t physical_width, float_t physical_height, float_t design_width, float_t design_height )
{
	if ( physical_width <= 0.f || physical_height <= 0.f || design_width <= 0.f || design_height <= 0.f )
	{
		return VirtualScreen{ design_width, design_height, 1.f };
	}

	const float_t scale = std::min( physical_width / design_width, physical_height / design_height );
	return VirtualScreen{ physical_width / scale, physical_height / scale, scale };
}

} // namespace core::ui
