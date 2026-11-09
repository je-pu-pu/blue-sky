#include "Stack.h"
#include "Theme.h"

#include <algorithm>

namespace core::ui
{

float_t Stack::get_spacing( const UIContext& ctx ) const
{
	return spacing_ >= 0.f ? spacing_ : ctx.theme.spacing;
}

float_t Stack::measure_height( float_t width, const UIContext& ctx ) const
{
	const float_t spacing = get_spacing( ctx );
	const float_t inner_w = std::max( 0.f, width - padding_ * 2.f );

	float_t total = 0.f;
	int visible = 0;

	if ( axis_ == Axis::HORIZONTAL )
	{
		int n = 0;
		for ( size_t i = 0; i < child_count(); i++ ) if ( child_at( i )->is_visible() ) n++;

		const float_t child_w = n > 0 ? ( inner_w - spacing * static_cast< float_t >( n - 1 ) ) / static_cast< float_t >( n ) : 0.f;

		for ( size_t i = 0; i < child_count(); i++ )
		{
			const Widget* c = child_at( i );
			if ( c->is_visible() ) total = std::max( total, c->measure_height( child_w, ctx ) );
		}

		return total + padding_ * 2.f;
	}

	for ( size_t i = 0; i < child_count(); i++ )
	{
		const Widget* c = child_at( i );

		if ( ! c->is_visible() )
		{
			continue;
		}

		total += c->measure_height( inner_w, ctx );
		visible++;
	}

	if ( visible > 1 )
	{
		total += spacing * static_cast< float_t >( visible - 1 );
	}

	return total + padding_ * 2.f;
}

void Stack::arrange( const Rect& rect, const UIContext& ctx )
{
	rect_ = rect;

	const float_t spacing = get_spacing( ctx );
	const Rect inner = rect.inset( padding_ );

	if ( axis_ == Axis::HORIZONTAL )
	{
		int n = 0;
		for ( size_t i = 0; i < child_count(); i++ ) if ( child_at( i )->is_visible() ) n++;

		if ( n == 0 )
		{
			return;
		}

		const float_t child_w = ( inner.w - spacing * static_cast< float_t >( n - 1 ) ) / static_cast< float_t >( n );
		float_t x = inner.x;

		for ( size_t i = 0; i < child_count(); i++ )
		{
			Widget* c = child_at( i );

			if ( ! c->is_visible() )
			{
				continue;
			}

			c->arrange( Rect( x, inner.y, child_w, inner.h ), ctx );
			x += child_w + spacing;
		}

		return;
	}

	// 伸ばす子の高さ = 全体 - 他の子の高さ - 間隔
	float_t flex_h = 0.f;

	if ( flex_ && flex_->is_visible() && index_of_child( flex_ ) >= 0 )
	{
		float_t others = 0.f;
		int visible = 0;

		for ( size_t i = 0; i < child_count(); i++ )
		{
			const Widget* c = child_at( i );
			if ( ! c->is_visible() ) continue;
			visible++;
			if ( c != flex_ ) others += c->measure_height( inner.w, ctx );
		}

		flex_h = std::max( 0.f, inner.h - others - spacing * static_cast< float_t >( std::max( 0, visible - 1 ) ) );
	}

	float_t y = inner.y;

	for ( size_t i = 0; i < child_count(); i++ )
	{
		Widget* c = child_at( i );

		if ( ! c->is_visible() )
		{
			continue;
		}

		const float_t h = c == flex_ ? flex_h : c->measure_height( inner.w, ctx );
		c->arrange( Rect( inner.x, y, inner.w, h ), ctx );
		y += h + spacing;
	}
}

} // namespace core::ui
