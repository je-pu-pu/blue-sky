#pragma once

#include <core/graphics/TextStyle.h>

namespace core::graphics
{

/**
 * 文字の描画（抽象層）
 *
 * 文字を積み（draw_text）、まとめて描く（flush）。座標は描画先のピクセル。
 * 実装は MSDF の文字の描画（MsdfTextRenderer。Direct3D 11）。
 * 以前は文字の描画が抽象層に無く、ゲーム側（blue_sky の GraphicsManager）からしか描けなかった（設計上の問題 B19）。
 * core 層は GraphicsManager::get_text_renderer() から得る。
 */
class TextRenderer
{
public:
	virtual ~TextRenderer() = default;

	/// フォントの読み込みと描画の準備ができているか
	virtual bool is_ready() const = 0;

	/// 文字を積む（左上の位置。改行に対応）
	virtual void draw_text( float x, float y, const char* text, const TextStyle& style ) = 0;
	virtual void draw_text( float x, float y, const wchar_t* text, const TextStyle& style ) = 0;

	/// 最初の行の幅
	virtual float measure_text_width( const char* text, float font_size ) = 0;
	virtual float measure_text_width( const wchar_t* text, float font_size ) = 0;

	/// 全行の高さ
	virtual float measure_text_height( const char* text, float font_size ) = 0;
	virtual float measure_text_height( const wchar_t* text, float font_size ) = 0;

	/// 積んだ文字を描く
	virtual void flush() = 0;

	/// 代わりのフォントを足す（主のフォントに無い文字に使う。対応していなければ false）
	virtual bool add_fallback_font( const char* ) { return false; }

}; // class TextRenderer

} // namespace core::graphics
