#pragma once

#include <core/ui/Widget.h>

namespace core::ui
{

/**
 * 子を縦または横に並べるコンテナ
 *
 * - 縦: 子の希望の高さで上から順に並べる
 * - 横: 幅を等分して並べる（高さは最も高い子に揃える）
 * - 非表示の子は詰める
 * - ナビゲーションは並び順。wrap を指定すると、祖先へ抜けられないときだけ反対端へ回る
 */
class Stack : public Widget
{
private:
	Axis axis_;
	float_t spacing_ = -1.f;	///< 負なら Theme::spacing
	float_t padding_ = 0.f;
	bool wrap_ = false;
	const Widget* flex_ = nullptr;	///< 縦のとき、残りの高さをすべて与える子（nullptr = なし）

	float_t get_spacing( const UIContext& ctx ) const;

public:
	explicit Stack( Axis axis = Axis::VERTICAL ) : axis_( axis ) { }

	void set_spacing( float_t s ) { spacing_ = s; }
	void set_padding( float_t p ) { padding_ = p; }
	void set_wrap( bool w ) { wrap_ = w; }

	/// 縦のとき、指定した子に残りの高さを与える（タブの中身を広げ、ボタン行を下端に置く等）
	void set_flex( const Widget* child ) { flex_ = child; }

	Axis nav_axis() const override { return axis_; }
	bool nav_wrap() const override { return wrap_; }

	float_t measure_height( float_t width, const UIContext& ctx ) const override;
	void arrange( const Rect& rect, const UIContext& ctx ) override;

}; // class Stack

} // namespace core::ui
