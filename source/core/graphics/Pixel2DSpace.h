#pragma once

#include <core/type.h>

namespace core::graphics
{

/**
 * 2D 描画（Sprite・文字）の座標系：描画先のピクセル座標 → 描画用の座標
 *
 * 以前は Sprite と文字の描画が同じ変換をそれぞれ書いていて、どちらもスワップチェーンの大きさで決め打ちしていた。
 * そのため、画面と違う大きさの描画先（VR の目ごとのバッファなど）では位置がずれ、
 * 立体視のずらし（offset_x）は Sprite にしか効かなかった（設計上の問題 B16）。
 *
 * 描画用の座標は「高さを 2 とした座標」で、縦は -1〜+1、横は -aspect〜+aspect。
 * これを正射影（幅 2 × aspect、高さ 2）で画面に写し、最後に offset_x だけ横にずらす。
 */
struct Pixel2DSpace
{
	float_t width = 0.f;		///< 描画先の幅（ピクセル）
	float_t height = 0.f;		///< 描画先の高さ（ピクセル）
	float_t offset_x = 0.f;		///< 立体視のずらし（正射影の後の座標で）

	/// 描画できる大きさか（最小化中などは 0）
	bool is_valid() const { return width > 0.f && height > 0.f; }

	float_t aspect() const { return width / height; }

	/// ピクセルの x → 描画用の座標
	float_t to_view_x( float_t px ) const { return px * 2.f / height - aspect(); }

	/// ピクセルの y → 描画用の座標（下が負）
	float_t to_view_y( float_t py ) const { return -( py * 2.f / height - 1.f ); }

	/// 正射影の幅（Matrix::set_orthographic に渡す）
	float_t ortho_width() const { return 2.f * aspect(); }

	/// 正射影の高さ
	static constexpr float_t ortho_height() { return 2.f; }
};

} // namespace core::graphics
