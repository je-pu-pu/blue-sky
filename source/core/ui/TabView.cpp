#include "TabView.h"
#include "TextFit.h"
#include "Renderer.h"
#include "Screen.h"
#include "Theme.h"

#include <algorithm>

namespace core::ui
{

void TabView::select( size_t index, UIContext& ctx )
{
	if ( index >= children_.size() || index == current_ )
	{
		return;
	}

	current_ = index;
	ctx.screen.invalidate_layout();
	ctx.screen.play( UISoundEvent::MOVE );

	// ページの入口へフォーカス（ページを覚えていればその位置）
	Widget* page = children_[ current_ ].get();
	Screen* screen = & ctx.screen;
	ctx.screen.post( [screen, page] { screen->focus_into( page ); } );
}

float_t TabView::measure_height( float_t width, const UIContext& ctx ) const
{
	const float_t page_h = children_.empty() ? 0.f : children_[ current_ ]->measure_height( width, ctx );
	return text_row_height( ctx.renderer, ctx.theme.tab_height, ctx.theme.font_size_normal, ctx.theme.row_text_padding ) + ctx.theme.spacing * 2.f + page_h;
}

void TabView::arrange( const Rect& rect, const UIContext& ctx )
{
	rect_ = rect;

	const auto& t = ctx.theme;
	const size_t n = children_.size();

	tab_rects_.clear();

	if ( n == 0 )
	{
		return;
	}

	const float_t tab_w = ( rect.w - t.spacing * static_cast< float_t >( n - 1 ) ) / static_cast< float_t >( n );

	for ( size_t i = 0; i < n; i++ )
	{
		tab_rects_.emplace_back( rect.x + ( tab_w + t.spacing ) * static_cast< float_t >( i ), rect.y, tab_w, text_row_height( ctx.renderer, t.tab_height, t.font_size_normal, t.row_text_padding ) );
	}

	// 全ページを配置しておく（切り替え直後にフォーカス・スクロールが正しい矩形を使えるように）
	const float_t top = rect.y + text_row_height( ctx.renderer, t.tab_height, t.font_size_normal, t.row_text_padding ) + t.spacing * 2.f;
	const Rect page_rect( rect.x, top, rect.w, std::max( 0.f, rect.bottom() - top ) );

	for ( auto& page : children_ )
	{
		page->arrange( page_rect, ctx );
	}
}

bool TabView::on_tab( int direction, UIContext& ctx )
{
	const size_t n = children_.size();

	if ( n < 2 || direction == 0 )
	{
		return false;
	}

	// 端では反対端へ回る
	select( ( current_ + ( direction > 0 ? 1 : n - 1 ) ) % n, ctx );
	return true;
}

bool TabView::on_pointer( const PointerEvent& e, UIContext& ctx )
{
	if ( e.type == PointerEvent::Type::PRESS )
	{
		pressed_ = true;
		return true;
	}

	if ( e.type == PointerEvent::Type::RELEASE )
	{
		if ( pressed_ && e.inside )
		{
			for ( size_t i = 0; i < tab_rects_.size(); i++ )
			{
				if ( tab_rects_[ i ].contains( e.x, e.y ) )
				{
					select( i, ctx );
					break;
				}
			}
		}

		pressed_ = false;
		return true;
	}

	return true;
}

void TabView::render( Renderer& renderer, const UIContext& ctx ) const
{
	const auto& t = ctx.theme;

	for ( size_t i = 0; i < tab_rects_.size(); i++ )
	{
		const bool selected = i == current_;

		draw_brush( renderer, tab_rects_[ i ], selected ? t.row_focused : t.row );

		if ( selected )
		{
			draw_brush( renderer, Rect( tab_rects_[ i ].x, tab_rects_[ i ].bottom() - t.accent_bar_thickness, tab_rects_[ i ].w, t.accent_bar_thickness ), pick_brush( renderer, t.tab_indicator, t.accent ) );
		}

		draw_text_fit( renderer, tab_rects_[ i ], ctx.tr( titles_[ i ] ), make_text_style( selected ? t.text_focused : t.text_dim, t.font_size_normal, core::graphics::HAlign::CENTER ) );

		// 印 : 見出しの右上に丸
		if ( has_badge( i ) )
		{
			const float_t s = t.tab_badge_size;
			const Rect& r = tab_rects_[ i ];
			draw_brush( renderer, Rect( r.right() - s * 1.5f, r.y + s * 0.5f, s, s ), pick_brush( renderer, t.tab_badge, t.accent ) );
		}
	}

	render_children( renderer, ctx );
}

} // namespace core::ui
