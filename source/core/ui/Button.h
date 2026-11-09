#pragma once

#include <core/ui/Widget.h>

#include <functional>

namespace core::ui
{

/**
 * ボタン
 *
 * 決定（Enter / パッド A）か、部品の上で押して部品の上で離したクリックで on_click を呼ぶ。
 * 部品の外のクリックには反応しない。
 */
class Button : public Widget
{
private:
	string_t text_key_;
	std::function< void() > on_click_;
	bool pressed_ = false;

	void click( UIContext& ctx );

public:
	/// 捕捉が外れた（層が積まれたなど）: 押している・ドラッグ中の状態を解除する
	void on_capture_lost() override { pressed_ = false; }

	Button( const string_t& text_key, std::function< void() > on_click )
		: text_key_( text_key ), on_click_( std::move( on_click ) ) { }

	void set_text( const string_t& key ) { text_key_ = key; }
	const string_t& get_text() const { return text_key_; }
	void set_on_click( std::function< void() > f ) { on_click_ = std::move( f ); }

	bool is_focusable_type() const override { return true; }

	float_t measure_height( float_t width, const UIContext& ctx ) const override;

	bool on_confirm( UIContext& ctx ) override;
	bool on_pointer( const PointerEvent& e, UIContext& ctx ) override;

	void render( Renderer& renderer, const UIContext& ctx ) const override;
	void collect_hints( std::vector< Hint >& hints ) const override;

}; // class Button

} // namespace core::ui
