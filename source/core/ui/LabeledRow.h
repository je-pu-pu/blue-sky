#pragma once

#include <core/ui/Widget.h>

namespace core::ui
{

/**
 * 「左にラベル、右に値」の 1 行の基底（Toggle / Slider / Choice / KeyBind）
 *
 * 行の背景・ラベルの描画と、値エリアの矩形計算をまとめる。
 */
class LabeledRow : public Widget
{
protected:
	string_t label_key_;
	Rect label_rect_;
	Rect value_rect_;

	/// 行の背景とラベルを描く
	void render_row( Renderer& renderer, const UIContext& ctx ) const;

	/// 値の文字色（フォーカス・無効を反映）
	Color value_color( const UIContext& ctx ) const;

public:
	explicit LabeledRow( const string_t& label_key ) : label_key_( label_key ) { }

	bool is_focusable_type() const override { return true; }

	float_t measure_height( float_t width, const UIContext& ctx ) const override;
	void arrange( const Rect& rect, const UIContext& ctx ) override;

	const Rect& get_value_rect() const { return value_rect_; }

}; // class LabeledRow

} // namespace core::ui
