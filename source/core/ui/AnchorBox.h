#pragma once

#include <core/ui/Widget.h>

namespace core::ui
{

/// 画面（親の矩形）のどこを基準に置くか
enum class Anchor
{
	TOP_LEFT,		TOP,		TOP_RIGHT,
	LEFT,			CENTER,		RIGHT,
	BOTTOM_LEFT,	BOTTOM,		BOTTOM_RIGHT
};

/**
 * 子を 1 つ、親の矩形の端や角を基準に置くコンテナ（HUD・トースト・ツールチップの位置決め）
 *
 * 子の幅は set_width（0 以下なら親の幅いっぱい）、高さは子の measure_height。
 * 端からの間隔は set_margin。画面の端に寄せる HUD は、Screen のセーフエリアの内側に置かれる。
 * 以前は位置の基準がなく、パネルの中央寄せしかできなかった（設計上の問題 B13・B7）。
 */
class AnchorBox : public Widget
{
private:
	Anchor anchor_;
	float_t width_ = 0.f;
	float_t margin_ = 0.f;

public:
	explicit AnchorBox( Anchor anchor ) : anchor_( anchor ) { }

	void set_anchor( Anchor a ) { anchor_ = a; }
	void set_width( float_t w ) { width_ = w; }
	void set_margin( float_t m ) { margin_ = m; }

	/// 親の矩形の中での、子の矩形（子の大きさと基準から）
	static Rect place( const Rect& parent, float_t w, float_t h, Anchor anchor, float_t margin );

	float_t measure_height( float_t width, const UIContext& ctx ) const override;
	void arrange( const Rect& rect, const UIContext& ctx ) override;

}; // class AnchorBox

} // namespace core::ui
