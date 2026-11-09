#pragma once

#include <core/ui/Widget.h>
#include <core/graphics/TextStyle.h>

namespace core::ui
{

/**
 * テキスト表示（見出し・説明文など）。フォーカスは受けない
 *
 * 表示文字列は「文字列キー」（文字列テーブルで翻訳。未登録ならそのまま表示）か、
 * 毎フレーム文字列を返す関数で与える。折り返し指定時は `\n` と幅で改行する。
 */
class Label : public Widget
{
public:
	enum class Style
	{
		TITLE,
		HEADER,
		NORMAL,
		SMALL
	};

private:
	string_t text_key_;
	std::function< string_t() > provider_;
	Style style_;
	bool wrap_ = false;
	core::graphics::HAlign align_ = core::graphics::HAlign::LEFT;

	float_t get_font_size( const UIContext& ) const;
	Color get_color( const UIContext& ) const;
	string_t get_text( const UIContext& ) const;

public:
	explicit Label( const string_t& text_key = string_t(), Style style = Style::NORMAL )
		: text_key_( text_key ), style_( style ) { }

	void set_text( const string_t& key ) { text_key_ = key; provider_ = nullptr; }
	void set_text_provider( std::function< string_t() > f ) { provider_ = std::move( f ); }
	void set_wrap( bool w ) { wrap_ = w; }
	void set_align( core::graphics::HAlign a ) { align_ = a; }

	float_t measure_height( float_t width, const UIContext& ctx ) const override;
	void render( Renderer& renderer, const UIContext& ctx ) const override;

	/**
	 * テキストを幅に収まるように行へ分ける
	 *
	 * `\n` で必ず改行し、幅を超える行は UTF-8 のコードポイント単位で貪欲に折り返す
	 * （日本語向け。英単語の途中でも折り返す）。
	 */
	static std::vector< string_t > wrap_lines( const string_t& text, float_t width, float_t font_size, const Renderer& renderer );

}; // class Label

} // namespace core::ui
