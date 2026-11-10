#include "Widget.h"
#include "StringTable.h"

#include <algorithm>

namespace core::ui
{

string_t UIContext::tr( const string_t& key ) const
{
	return strings.get( key );
}

Widget::~Widget() = default;

void Widget::remove_all_children()
{
	children_.clear();
	last_focused_child_ = nullptr;
}

int Widget::index_of_child( const Widget* child ) const
{
	for ( size_t i = 0; i < child_count(); i++ )
	{
		if ( child_at( i ) == child )
		{
			return static_cast< int >( i );
		}
	}

	return -1;
}

bool Widget::contains_widget( const Widget* w ) const
{
	for ( const Widget* p = w; p; p = p->parent_ )
	{
		if ( p == this )
		{
			return true;
		}
	}

	return false;
}

bool Widget::is_visible_in_tree() const
{
	for ( const Widget* p = this; p; p = p->parent_ )
	{
		if ( ! p->visible_ )
		{
			return false;
		}

		// 親が子として公開していない（TabView の非表示ページ等）なら見えない
		if ( p->parent_ && p->parent_->index_of_child( p ) < 0 )
		{
			return false;
		}
	}

	return true;
}

bool Widget::can_focus() const
{
	if ( ! is_focusable_type() || ! is_enabled() || ! is_visible_in_tree() )
	{
		return false;
	}

	// 祖先が無効なら子も無効
	for ( const Widget* p = parent_; p; p = p->parent_ )
	{
		if ( ! p->is_enabled() )
		{
			return false;
		}
	}

	return true;
}

Extent Widget::measure( const Extent& available, const UIContext& ctx ) const
{
	const float_t preferred = measure_width( ctx );
	const float_t w = preferred > 0.f ? std::min( preferred, available.w ) : available.w;

	return Extent( w, measure_height( w, ctx ) );
}

float_t Widget::measure_height( float_t, const UIContext& ) const
{
	return 0.f;
}

void Widget::arrange( const Rect& rect, const UIContext& )
{
	rect_ = rect;
}

bool Widget::on_action( ActionId action, UIContext& ctx )
{
	switch ( action )
	{
		case ui_action::NAV_UP:		return on_nav( NavDir::UP, ctx );
		case ui_action::NAV_DOWN:	return on_nav( NavDir::DOWN, ctx );
		case ui_action::NAV_LEFT:	return on_nav( NavDir::LEFT, ctx );
		case ui_action::NAV_RIGHT:	return on_nav( NavDir::RIGHT, ctx );
		case ui_action::CONFIRM:	return on_confirm( ctx );
		case ui_action::CANCEL:		return on_cancel( ctx );
		case ui_action::TAB_PREV:	return on_tab( -1, ctx );
		case ui_action::TAB_NEXT:	return on_tab( +1, ctx );
		default:					return false;
	}
}

void Widget::update( float_t dt, UIContext& ctx )
{
	for ( size_t i = 0; i < child_count(); i++ )
	{
		child_at( i )->update( dt, ctx );
	}
}

void Widget::render( Renderer& renderer, const UIContext& ctx ) const
{
	render_children( renderer, ctx );
}

void Widget::render_children( Renderer& renderer, const UIContext& ctx ) const
{
	for ( size_t i = 0; i < child_count(); i++ )
	{
		const Widget* child = child_at( i );

		if ( child->is_visible() )
		{
			child->render( renderer, ctx );
		}
	}
}

void Widget::render_clipped( Renderer& renderer, const UIContext& ctx, const Rect& clip ) const
{
	if ( ! visible_ )
	{
		return;
	}

	if ( is_inside( rect_, clip ) )
	{
		render( renderer, ctx );
		return;
	}

	// はみ出すコンテナは子へ降りる（はみ出す末端は描画しない）
	for ( size_t i = 0; i < child_count(); i++ )
	{
		child_at( i )->render_clipped( renderer, ctx, clip );
	}
}

} // namespace core::ui
