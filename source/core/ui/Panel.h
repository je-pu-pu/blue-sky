#pragma once

#include <core/ui/Widget.h>

namespace core::ui
{

/**
 * 画面の枠（背景の暗幕・パネル・タイトル・中身・説明欄・操作ヒント）
 *
 * 子は「中身」として 1 つ追加する（Stack や TabView）。
 * 説明欄にはフォーカス中の部品の説明文を、ヒント行には Screen が作る操作ヒントを表示する。
 */
class Panel : public Widget
{
private:
	string_t title_key_;
	float_t width_ = 0.f;		///< 0 なら Theme::panel_width
	bool fit_height_ = false;	///< 中身の高さに合わせる（false なら画面の高さいっぱい）
	bool dim_screen_ = true;
	bool show_description_ = true;

	Rect panel_rect_;
	Rect title_rect_;
	Rect description_rect_;
	Rect hint_rect_;

	Widget* get_body() const { return children_.empty() ? nullptr : children_.front().get(); }

public:
	explicit Panel( const string_t& title_key ) : title_key_( title_key ) { }

	void set_width( float_t w ) { width_ = w; }
	void set_fit_height( bool f ) { fit_height_ = f; }
	void set_dim_screen( bool d ) { dim_screen_ = d; }
	void set_show_description( bool s ) { show_description_ = s; }

	const Rect& get_panel_rect() const { return panel_rect_; }

	void arrange( const Rect& rect, const UIContext& ctx ) override;
	void render( Renderer& renderer, const UIContext& ctx ) const override;

}; // class Panel

} // namespace core::ui
