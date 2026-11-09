#pragma once

#include <core/ui/LabeledRow.h>

#include <functional>

namespace core::ui
{

/**
 * 1 行の文字入力（セーブデータの名前・プレイヤー名・検索など）
 *
 * - フォーカス中は文字を受け取る（UIInput::text）。IME で変換を確定した文字も届く
 * - IME で変換中の文字は、値の後ろに下線付きで見せる（UIInput::composition。以前は確定するまで何も見えなかった）
 * - バックスペースで末尾の 1 文字（コードポイント）を消す。カーソルは常に末尾
 * - 値は持たず、getter / setter（UTF-8）で読み書きする（ほかの部品と同じ）
 * - 決定（Enter）で on_submit を呼ぶ（設定されていれば）
 * - フォーカス中は Q / E のタブ切替をしない（文字として入力されるため）
 */
class TextField : public LabeledRow
{
public:
	using Getter = std::function< string_t() >;
	using Setter = std::function< void( const string_t& ) >;

private:
	Getter getter_;
	Setter setter_;
	std::function< void() > on_submit_;
	size_t max_length_ = 32;		///< 最大の文字数（コードポイント）
	float_t blink_time_ = 0.f;
	string_t composition_;			///< IME で変換中の文字（UTF-8）

public:
	TextField( const string_t& label_key, Getter getter, Setter setter )
		: LabeledRow( label_key ), getter_( std::move( getter ) ), setter_( std::move( setter ) ) { }

	void set_max_length( size_t n ) { max_length_ = n; }
	void set_on_submit( std::function< void() > f ) { on_submit_ = std::move( f ); }

	string_t get_value() const { return getter_ ? getter_() : string_t(); }

	bool wants_text_input() const override { return is_focused() && is_enabled(); }
	bool on_text( const std::u32string& text, UIContext& ctx ) override;
	void on_composition( const std::u32string& text, UIContext& ctx ) override;

	const string_t& get_composition() const { return composition_; }
	bool on_confirm( UIContext& ctx ) override;

	void update( float_t dt, UIContext& ctx ) override;
	void render( Renderer& renderer, const UIContext& ctx ) const override;
	void collect_hints( std::vector< Hint >& hints ) const override;

	/// UTF-8 の文字数（コードポイント数）
	static size_t count_codepoints( const string_t& s );

	/// UTF-8 の末尾の 1 文字（コードポイント）を消した文字列
	static string_t erase_last_codepoint( const string_t& s );

	/// コードポイントを UTF-8 にする（不正な値は空）
	static string_t to_utf8( char32_t c );

}; // class TextField

} // namespace core::ui
