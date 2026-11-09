#pragma once

#include <core/ui/LabeledRow.h>

#include <functional>
#include <vector>

namespace core::ui
{

/**
 * 選択肢を左右で送る部品（「< 1920 x 1080 >」）
 *
 * 値は選択肢の添字。現在値が選択肢に無い場合は getter が -1 を返し、
 * set_custom_text() で与えた文字列（例: 「カスタム (1366 x 768)」）を表示する。
 */
class Choice : public LabeledRow
{
public:
	/// 捕捉が外れた（層が積まれたなど）: 押している・ドラッグ中の状態を解除する
	void on_capture_lost() override { pressed_ = false; }

	using Getter = std::function< int() >;
	using Setter = std::function< void( int ) >;

private:
	Getter getter_;
	Setter setter_;
	std::vector< string_t > option_keys_;
	std::function< string_t() > custom_text_;
	bool wrap_ = false;
	bool pressed_ = false;

	Rect left_arrow_rect_;
	Rect right_arrow_rect_;

	void step( int delta, UIContext& ctx );

public:
	Choice( const string_t& label_key, std::vector< string_t > option_keys, Getter getter, Setter setter )
		: LabeledRow( label_key ), getter_( std::move( getter ) ), setter_( std::move( setter ) ), option_keys_( std::move( option_keys ) ) { }

	void set_options( std::vector< string_t > keys ) { option_keys_ = std::move( keys ); }
	void set_custom_text( std::function< string_t() > f ) { custom_text_ = std::move( f ); }
	void set_wrap( bool w ) { wrap_ = w; }

	int get_index() const { return getter_ ? getter_() : -1; }
	size_t get_option_count() const { return option_keys_.size(); }

	void arrange( const Rect& rect, const UIContext& ctx ) override;

	bool on_nav( NavDir dir, UIContext& ctx ) override;
	bool on_pointer( const PointerEvent& e, UIContext& ctx ) override;

	void render( Renderer& renderer, const UIContext& ctx ) const override;
	void collect_hints( std::vector< Hint >& hints ) const override;

	/// 現在の表示文字列
	string_t get_display_text( const UIContext& ctx ) const;

}; // class Choice

} // namespace core::ui
