#include "ScrollView.h"
#include "Renderer.h"
#include "Screen.h"
#include "Theme.h"

#include <algorithm>

namespace core::ui
{

float_t ScrollView::get_max_scroll() const
{
	return std::max( 0.f, content_height_ - rect_.h );
}

void ScrollView::clamp_scroll()
{
	scroll_ = std::clamp( scroll_, 0.f, get_max_scroll() );
}

void ScrollView::ensure_visible( const Rect& r )
{
	// r は現在の（スクロール済みの）座標。表示範囲からはみ出した分だけ動かす
	if ( r.y < rect_.y )
	{
		scroll_ -= rect_.y - r.y;
	}
	else if ( r.bottom() > rect_.bottom() )
	{
		scroll_ += r.bottom() - rect_.bottom();
	}

	clamp_scroll();
}

float_t ScrollView::measure_height( float_t width, const UIContext& ctx ) const
{
	// 中身の高さを希望する（親が小さい矩形を与えればスクロールする）
	return get_content() ? get_content()->measure_height( width - scrollbar_width_ * 2.f, ctx ) : 0.f;
}

void ScrollView::arrange( const Rect& rect, const UIContext& ctx )
{
	rect_ = rect;
	scroll_step_ = ( ctx.theme.row_height + ctx.theme.spacing ) * 2.f;

	Widget* content = get_content();

	if ( ! content )
	{
		content_height_ = 0.f;
		return;
	}

	// スクロールバーの分だけ右を空ける
	const float_t content_w = rect.w - scrollbar_width_ * 2.f;
	content_height_ = content->measure_height( content_w, ctx );
	clamp_scroll();

	content->arrange( Rect( rect.x, rect.y - scroll_, content_w, content_height_ ), ctx );
}

bool ScrollView::on_wheel( int notches, UIContext& ctx )
{
	if ( get_max_scroll() <= 0.f )
	{
		return false;
	}

	const float_t before = scroll_;
	scroll_ -= static_cast< float_t >( notches ) * scroll_step_;
	clamp_scroll();

	if ( scroll_ != before )
	{
		ctx.screen.invalidate_layout();
	}

	return true;
}

void ScrollView::render( Renderer& renderer, const UIContext& ctx ) const
{
	if ( Widget* content = get_content() )
	{
		content->render_clipped( renderer, ctx, rect_ );
	}

	// スクロールバー（内容がはみ出すときだけ）
	const float_t max_scroll = get_max_scroll();

	if ( max_scroll > 0.f && content_height_ > 0.f )
	{
		const float_t bar_x = rect_.right() - scrollbar_width_;
		const float_t thumb_h = std::max( 24.f, rect_.h * rect_.h / content_height_ );
		const float_t thumb_y = rect_.y + ( rect_.h - thumb_h ) * ( scroll_ / max_scroll );

		draw_brush( renderer, Rect( bar_x, rect_.y, scrollbar_width_, rect_.h ), ctx.theme.track );
		draw_brush( renderer, Rect( bar_x, thumb_y, scrollbar_width_, thumb_h ), pick_brush( renderer, ctx.theme.scroll_thumb, ctx.theme.accent ) );
	}
}

} // namespace core::ui
