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
		// 配置と同じ幅で高さを測る
		const std::vector< float_t > widths = compute_widths( inner_w, spacing, ctx );
		size_t k = 0;

		for ( size_t i = 0; i < child_count(); i++ )
		{
			const Widget* c = child_at( i );

			if ( c->is_visible() )
			{
				total = std::max( total, c->measure_height( widths[ k ], ctx ) );
				k++;
			}
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

		// 決まった高さの子はその高さ、それ以外は希望の高さ（FILL の子も、希望としては内容の高さ）
		const Size s = get_size( c );
		total += std::max( s.mode == SizeMode::FIXED ? s.value : c->measure_height( inner_w, ctx ), s.min );
		visible++;
	}

	if ( visible > 1 )
	{
		total += spacing * static_cast< float_t >( visible - 1 );
	}

	return total + padding_ * 2.f;
}

void Stack::set_size( const Widget* child, Size size )
{
	for ( auto& s : sizes_ )
	{
		if ( s.first == child )
		{
			s.second = size;
			return;
		}
	}

	sizes_.emplace_back( child, size );
}

Stack::Size Stack::get_size( const Widget* child ) const
{
	for ( const auto& s : sizes_ )
	{
		if ( s.first == child )
		{
			return s.second;
		}
	}

	if ( axis_ == Axis::VERTICAL )
	{
		return child == flex_ ? Size{ SizeMode::FILL, 1.f } : Size{ SizeMode::CONTENT };
	}

	return Size();
}

std::vector< float_t > Stack::compute_heights( float_t inner_w, float_t inner_h, float_t spacing, const UIContext& ctx ) const
{
	std::vector< const Widget* > visible;

	for ( size_t i = 0; i < child_count(); i++ )
	{
		if ( child_at( i )->is_visible() )
		{
			visible.push_back( child_at( i ) );
		}
	}

	std::vector< float_t > heights( visible.size(), 0.f );

	if ( visible.empty() )
	{
		return heights;
	}

	// 決まった高さ・内容の高さを先に取り、残りを FILL の子が重みで分ける
	float_t remaining = inner_h - spacing * static_cast< float_t >( visible.size() - 1 );
	float_t total_weight = 0.f;

	for ( size_t i = 0; i < visible.size(); i++ )
	{
		const Size s = get_size( visible[ i ] );

		if ( s.mode == SizeMode::FILL )
		{
			total_weight += std::max( s.value, 0.f );
			continue;
		}

		heights[ i ] = std::max( s.mode == SizeMode::FIXED ? s.value : visible[ i ]->measure_height( inner_w, ctx ), s.min );
		remaining -= heights[ i ];
	}

	for ( size_t i = 0; i < visible.size(); i++ )
	{
		const Size s = get_size( visible[ i ] );

		if ( s.mode == SizeMode::FILL )
		{
			const float_t share = total_weight > 0.f ? std::max( 0.f, remaining ) * std::max( s.value, 0.f ) / total_weight : 0.f;
			heights[ i ] = std::max( share, s.min );
		}
	}

	return heights;
}

std::vector< float_t > Stack::compute_widths( float_t inner_w, float_t spacing, const UIContext& ctx ) const
{
	std::vector< const Widget* > visible;

	for ( size_t i = 0; i < child_count(); i++ )
	{
		if ( child_at( i )->is_visible() )
		{
			visible.push_back( child_at( i ) );
		}
	}

	std::vector< float_t > widths( visible.size(), 0.f );

	if ( visible.empty() )
	{
		return widths;
	}

	// 決まった幅・内容に合わせた幅を先に取り、残りを重みで分ける
	float_t remaining = inner_w - spacing * static_cast< float_t >( visible.size() - 1 );
	float_t total_weight = 0.f;

	for ( size_t i = 0; i < visible.size(); i++ )
	{
		const Size s = get_size( visible[ i ] );

		if ( s.mode == SizeMode::FIXED )
		{
			widths[ i ] = std::max( s.value, s.min );
		}
		else if ( s.mode == SizeMode::CONTENT )
		{
			widths[ i ] = std::max( visible[ i ]->measure_width( ctx ), s.min );
		}
		else
		{
			total_weight += std::max( s.value, 0.f );
			continue;
		}

		remaining -= widths[ i ];
	}

	for ( size_t i = 0; i < visible.size(); i++ )
	{
		const Size s = get_size( visible[ i ] );

		if ( s.mode == SizeMode::FILL )
		{
			const float_t share = total_weight > 0.f ? std::max( 0.f, remaining ) * std::max( s.value, 0.f ) / total_weight : 0.f;
			widths[ i ] = std::max( share, s.min );
		}
	}

	return widths;
}

void Stack::arrange( const Rect& rect, const UIContext& ctx )
{
	rect_ = rect;

	const float_t spacing = get_spacing( ctx );
	const Rect inner = rect.inset( padding_ );

	if ( axis_ == Axis::HORIZONTAL )
	{
		const std::vector< float_t > widths = compute_widths( inner.w, spacing, ctx );
		float_t x = inner.x;
		size_t k = 0;

		for ( size_t i = 0; i < child_count(); i++ )
		{
			Widget* c = child_at( i );

			if ( ! c->is_visible() )
			{
				continue;
			}

			c->arrange( Rect( x, inner.y, widths[ k ], inner.h ), ctx );
			x += widths[ k ] + spacing;
			k++;
		}

		return;
	}

	const std::vector< float_t > heights = compute_heights( inner.w, inner.h, spacing, ctx );
	float_t y = inner.y;
	size_t k = 0;

	for ( size_t i = 0; i < child_count(); i++ )
	{
		Widget* c = child_at( i );

		if ( ! c->is_visible() )
		{
			continue;
		}

		c->arrange( Rect( inner.x, y, inner.w, heights[ k ] ), ctx );
		y += heights[ k ] + spacing;
		k++;
	}
}

} // namespace core::ui
