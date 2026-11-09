#pragma once

#include <core/ui/Widget.h>
#include <core/ui/Brush.h>

namespace core::ui
{

/**
 * 「左にラベル、右に値」の 1 行の基底（Toggle / Slider / Choice / KeyBind / TextField）
 *
 * 行の背景・ラベルの描画と、値エリアの矩形計算をまとめる。
 * 行の組み立ては設定で変えられる（以前はラベルが必ずあり、アイコンは付けられなかった。設計上の問題 B9）:
 * - ラベルのキーが空なら、ラベルを描かず値の欄が行の幅いっぱいになる（ラベルのないスライダーなど）
 * - set_icon でラベルの左にアイコンを付ける
 */
class LabeledRow : public Widget
{
protected:
	string_t label_key_;
	Rect label_rect_;
	Rect value_rect_;
	Rect icon_rect_;
	Brush icon_;

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
	const Rect& get_label_rect() const { return label_rect_; }
	const Rect& get_icon_rect() const { return icon_rect_; }

	/// ラベルの左に付けるアイコン（画像や色）。既定はなし
	void set_icon( const Brush& icon ) { icon_ = icon; }

	/// ラベルを変える（空ならラベルなし）
	void set_label( const string_t& label_key ) { label_key_ = label_key; }
	bool has_label() const { return ! label_key_.empty(); }
	AccessibleInfo describe( const UIContext& ctx ) const override;

}; // class LabeledRow

} // namespace core::ui
