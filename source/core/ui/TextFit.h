#pragma once

#include <core/type.h>
#include <core/graphics/TextStyle.h>
#include <core/ui/Rect.h>

namespace core::ui
{
	class Renderer;

/// 文字を小さくするときの下限（元の大きさに対する比率）。これより小さくすると読めないので、末尾を省略する
inline constexpr float_t MIN_TEXT_FIT_SCALE = 0.6f;

/**
 * 1 行の文字を、矩形の幅に収まる文字の大きさと内容にする
 *
 * - 幅に収まれば、そのまま
 * - 収まらなければ、収まるまで文字を小さくする（元の MIN_TEXT_FIT_SCALE 倍まで）
 * - それでも収まらなければ、末尾を「…」にして省略する
 *
 * 言語や文字の大きさの設定で文字が長くなっても、隣の部品に重ならないようにする（設計上の問題 B3）。
 *
 * @param text [in/out] 表示する文字（省略したときは書き換える）
 * @param style [in/out] 文字のスタイル（小さくしたときは font_size を書き換える）
 */
void fit_text_to_width( const Renderer& renderer, float_t width, string_t& text, core::graphics::TextStyle& style );

/// 1 行の文字を矩形に収めて描く（fit_text_to_width を通してから描く）
void draw_text_fit( Renderer& renderer, const Rect& rect, string_t text, core::graphics::TextStyle style );

/**
 * 1 行の文字を入れる行の高さ（テーマの高さと、「文字の行の高さ＋余白」の大きい方）
 *
 * 文字を大きくしても、題名・タブ・行から文字がはみ出さないようにする。
 */
float_t text_row_height( const Renderer& renderer, float_t min_height, float_t font_size, float_t padding );

} // namespace core::ui
