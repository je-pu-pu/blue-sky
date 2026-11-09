#pragma once

#include <core/ui/Screen.h>
#include <core/ui/Widget.h>

namespace core::ui
{
	class Button;
	class Label;

/**
 * モーダルダイアログ（Screen::show_dialog() が生成する）
 *
 * メッセージとボタン行を持つ。ボタンが押されたら、
 * ① ボタンのコールバック（値のコピー）を post ② 自分の層を閉じる処理を post の順に積む。
 * 層の閉じ方は「自分の層を指定して閉じる」なので、コールバックが次のダイアログを開いても巻き込まない。
 */
class Dialog : public Widget
{
private:
	DialogSpec spec_;
	Screen* screen_ = nullptr;
	Label* message_ = nullptr;
	std::vector< Button* > buttons_;
	float_t remaining_ = 0.f;
	bool closed_ = false;

public:
	explicit Dialog( DialogSpec spec );

	/// show_dialog() が設定する
	void set_screen( Screen* s ) { screen_ = s; }

	/// index 番目のボタンを押したことにする（2 回目以降は無視）
	void press( int index );

	/// キャンセル扱い（cancel_index のボタン）。キャンセル不可なら何もしない
	void press_cancel();

	Widget* get_default_button() const;
	float_t get_remaining_time() const { return remaining_; }
	bool is_closed() const { return closed_; }

	float_t measure_height( float_t width, const UIContext& ctx ) const override;
	void arrange( const Rect& rect, const UIContext& ctx ) override;
	void update( float_t dt, UIContext& ctx ) override;
	void render( Renderer& renderer, const UIContext& ctx ) const override;

}; // class Dialog

} // namespace core::ui
