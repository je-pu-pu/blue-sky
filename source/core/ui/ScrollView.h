#pragma once

#include <core/ui/Widget.h>

namespace core::ui
{

/**
 * 縦スクロールする表示範囲（子は 1 つ。通常は縦の Stack）
 *
 * - 方向キーでフォーカスが移ると、その部品が見える位置まで自動でスクロールする
 *   （マウスのホバーではスクロールしない）
 * - ホイールでスクロールする
 * - 表示範囲に完全に収まる部品だけを描画・ヒット判定する（行が途中で欠けて見えることはない）
 * - ナビゲーションは素通し（中の Stack の並び順で移動する）
 */
class ScrollView : public Widget
{
private:
	float_t scroll_ = 0.f;			///< 内容の先頭からのスクロール量
	float_t content_height_ = 0.f;
	float_t scroll_step_ = 0.f;		///< ホイール 1 ノッチの量（arrange で Theme から決める）

	Widget* get_content() const { return children_.empty() ? nullptr : children_.front().get(); }
	void clamp_scroll();

public:
	float_t get_scroll() const { return scroll_; }
	float_t get_content_height() const { return content_height_; }
	float_t get_max_scroll() const;

	const Rect* get_view_clip() const override { return & rect_; }
	bool ensure_visible( const Rect& r ) override;

	float_t measure_height( float_t width, const UIContext& ctx ) const override;
	void arrange( const Rect& rect, const UIContext& ctx ) override;

	bool on_wheel( int notches, UIContext& ctx ) override;

	void render( Renderer& renderer, const UIContext& ctx ) const override;

}; // class ScrollView

} // namespace core::ui
