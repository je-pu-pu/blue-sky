#pragma once

#include <core/type.h>
#include <core/ui/Rect.h>

namespace core::ui
{
	class Renderer;

/// 四辺の幅（9 分割の枠幅など）
struct Insets
{
	float_t left = 0.f;
	float_t top = 0.f;
	float_t right = 0.f;
	float_t bottom = 0.f;

	Insets() = default;
	Insets( float_t l, float_t t, float_t r, float_t b ) : left( l ), top( t ), right( r ), bottom( b ) { }

	bool is_zero() const { return left == 0.f && top == 0.f && right == 0.f && bottom == 0.f; }
};

/// 9 分割の辺・中央の埋め方
enum class FillMode
{
	STRETCH,	///< 引き伸ばす（単色・グラデーションの枠向け）
	REPEAT,		///< 等倍で並べる。端数は両端で欠ける（中央寄せ）
	ROUND,		///< 並べる枚数を整数にし、わずかに伸縮させてぴったり収める（模様のある枠向け）
	NONE,		///< 描かない（中央だけに使える。枠だけを描き、中身は透かす）
};

/**
 * 矩形の塗り方（単色または画像）
 *
 * - image が空なら color で塗る（color の既定値は透明なので、何も設定しなければ何も描かない）。
 * - image があれば画像を描き、tint を掛け合わせる。画像が読めないときは color で塗る（color が透明なら何も描かない）。
 * - slice が 0 でなければ 9 分割で描く。四隅は伸ばさない。辺は辺の方向に、中央は両方向に、
 *   edge / center の指定で埋める（引き伸ばす・並べる・整数枚に合わせて並べる）。模様のある枠は伸ばすと崩れるので並べる。
 *   slice は元画像のピクセル単位。描画先での枠幅は slice * slice_scale（仮想座標）。
 *   描画先が枠幅の合計より小さいときは、枠幅を比率を保って縮める。
 *   並べるときの 1 枚の大きさは、元画像の辺・中央の大きさに、枠と同じ倍率を掛けたもの（模様の縦横比を保つ）。
 * - source の幅が 0 なら画像全体を使う。0 でなければ画像の一部（ピクセル単位）を切り出して使う（1 枚の画像に複数の部品を詰める場合）。
 *
 * Color から暗黙に変換できるので、単色の指定はこれまでどおり Color を代入すればよい。
 */
struct Brush
{
	Color color;							///< 単色の塗り。画像があるときは読めなかった場合の代わり
	string_t image;							///< 画像名（Renderer が解決する）
	Color tint = Color( 1.f, 1.f, 1.f, 1.f );	///< 画像に掛け合わせる色
	Rect source;							///< 切り出し範囲（ピクセル）。幅 0 なら画像全体
	Insets slice;							///< 9 分割の枠幅（元画像のピクセル）
	float_t slice_scale = 1.f;				///< 枠幅の倍率（元画像 1 ピクセルが仮想座標で何ピクセルか）
	FillMode edge = FillMode::STRETCH;		///< 9 分割の四辺の埋め方
	FillMode center = FillMode::STRETCH;	///< 9 分割の中央の埋め方

	Brush() = default;
	Brush( const Color& c ) : color( c ) { }		// 暗黙変換を許す（テーマの既定値を色で書けるように）

	static Brush from_image( const string_t& image_name, const Insets& slice_insets = Insets(), const Color& tint_color = Color( 1.f, 1.f, 1.f, 1.f ) )
	{
		Brush b;
		b.image = image_name;
		b.slice = slice_insets;
		b.tint = tint_color;
		return b;
	}

	bool has_image() const { return ! image.empty(); }

	/// 何かを描く設定か（画像がある、または色が透明でない）
	bool is_visible() const { return has_image() || color.a() > 0.f; }

}; // struct Brush

/**
 * Brush で矩形を塗る
 *
 * 9 分割の分解・切り出し範囲の解決・画像が読めないときの単色への切り替えはここで行う。
 * Renderer には、分解済みの単純な draw_image / draw_rect だけを依頼する。
 *
 * - 9 分割は外周を物理ピクセルにそろえ、枠幅も整数ピクセルにそろえてから分ける（角の太さが位置で変わらない）。
 * - clip を渡すと、その範囲だけを描く（画像は元画像の範囲も同じ比率で切る）。
 *   スライダーの充填のように「全体の形のまま一部だけ見せる」用途に使う（形を縮めると角が潰れるため）。
 */
void draw_brush( Renderer& renderer, const Rect& dst, const Brush& brush, const Rect* clip = nullptr );

/**
 * フォーカスの印（左端の帯）を描く
 *
 * フォーカスを背景の色の違いだけで示すと、色の見分けにくい人には分からない。形（帯）でも示す。
 * 幅はテーマの focus_marker_width（0 なら描かない）、色は accent。
 */
void draw_focus_marker( Renderer& renderer, const Rect& rect, const struct Theme& theme );

/// 実際に何かが描かれるか（画像が読める、または色が透明でない）。has_image() と違い、読めない画像は描けない扱い
bool can_draw( const Renderer& renderer, const Brush& brush );

/// preferred が描けるならそれを、描けなければ fallback を返す（用途別の面が未指定のとき共通の面で代用する）
const Brush& pick_brush( const Renderer& renderer, const Brush& preferred, const Brush& fallback );

} // namespace core::ui
