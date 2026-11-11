#pragma once

#include <win/Rect.h>
#include <core/type.h>

namespace win
{
	class Rect;
	class Point;
}

namespace core::graphics
{
	class Texture;

/**
 * 2D の四角形を描く（画面のピクセル座標）
 *
 * begin() 〜 end() の間で draw 系を呼ぶ。
 * 切り出し範囲を整数ピクセルの Rect で指定する draw() のほか、
 * ピクセル未満の精度が要るとき（UI の画像の切り取りなど）は UV で指定する draw_uv() を使う。
 */
class Sprite
{
public:
	using Texture		= Texture;

	using Rect			= win::Rect;
	using Point			= win::Point;

	/// テクスチャ座標の範囲（0〜1。左上 u0, v0 / 右下 u1, v1）
	struct UVRect
	{
		float_t u0 = 0.f;
		float_t v0 = 0.f;
		float_t u1 = 1.f;
		float_t v1 = 1.f;
	};

	/// テクスチャのアルファの持ち方
	enum class AlphaMode
	{
		STRAIGHT,		///< ストレート（通常の読み込み。SRC_ALPHA / INV_SRC_ALPHA で合成）
		PREMULTIPLIED,	///< 乗算済み（UI 用の読み込み。ONE / INV_SRC_ALPHA で合成）
	};

protected:
	static const Color&	White;

	virtual void draw( const Rect*, const Texture*, const Rect*, const Color* ) = 0;
	virtual void draw_uv_impl( const Rect& dst, const Texture* t, const UVRect& uv, const Color& c, AlphaMode mode ) = 0;

public:
	explicit Sprite() { }
	virtual ~Sprite() { }

	virtual void set_transform( const Matrix& ) = 0;
	virtual void set_ortho_offset( float_t ortho_offset ) = 0;

	virtual void begin() = 0;

	void draw( const Point& dp, const Texture* t, const Rect& sr, const Color& c = White )
	{
		Rect dr = Rect::Size( dp.x(), dp.y(), sr.width(), sr.height() );
		draw( & dr, t, & sr, & c );
	}

	void draw( const Rect& dr, const Texture* t, const Rect& sr, const Color& c = White )
	{
		draw( & dr, t, & sr, & c );
	}

	void draw( const Rect& dr, const Texture* t, const Color& c = White )
	{
		draw( & dr, t, 0, & c );
	}

	void draw( const Texture* t, const Rect& sr, const Color& c = White )
	{
		draw( 0, t, & sr, & c );
	}

	void draw( const Texture* t, const Color& c = White )
	{
		draw( 0, t, 0, & c );
	}

	/// テクスチャの UV 範囲を描画先の矩形に描く（色を掛け合わせる）。テクスチャが nullptr なら何もしない
	/// 色はストレート（乗算前）で渡す。乗算済みのテクスチャでも、掛け合わせは正しく行われる
	void draw_uv( const Rect& dst, const Texture* t, const UVRect& uv, const Color& c = White, AlphaMode mode = AlphaMode::STRAIGHT )
	{
		draw_uv_impl( dst, t, uv, c, mode );
	}

	/// テクスチャなしで塗りつぶし矩形を描画
	virtual void draw_color( const Rect&, const Color& ) = 0;

	virtual void end() = 0;

	/**
	 * 溜めている四角形を描く
	 *
	 * Sprite は同じテクスチャ・同じ描き方の四角形を溜めて 1 回で描く（設計上の問題 B15）。
	 * Sprite 以外のもの（文字・3D）を begin と end の間に描く前には、順番が崩れないよう必ず呼ぶ
	 * （GraphicsManager の文字の描画・render_technique は自分で呼ぶ）。
	 */
	virtual void flush() { }

}; // class Sprite

} // namespace core::graphics
