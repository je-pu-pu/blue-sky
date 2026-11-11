#include "Grid.h"
#include "Theme.h"

#include <algorithm>

namespace core::ui
{

float_t Grid::get_spacing( const UIContext& ctx ) const
{
	return spacing_ >= 0.f ? spacing_ : ctx.theme.spacing;
}

float_t Grid::measure_height( float_t width, const UIContext& ctx ) const
{
	const float_t spacing = get_spacing( ctx );
	const float_t cell_w = ( width - spacing * static_cast< float_t >( columns_ - 1 ) ) / static_cast< float_t >( columns_ );
	const size_t n = children_.size();

	float_t total = 0.f;
	int rows = 0;

	for ( size_t start = 0; start < n; start += static_cast< size_t >( columns_ ) )
	{
		float_t row_h = 0.f;

		for ( size_t i = start; i < std::min( n, start + static_cast< size_t >( columns_ ) ); i++ )
		{
			if ( children_[ i ]->is_visible() )
			{
				row_h = std::max( row_h, children_[ i ]->measure_height( cell_w, ctx ) );
			}
		}

		total += row_h;
		rows++;
	}

	return total + spacing * static_cast< float_t >( std::max( rows - 1, 0 ) );
}

void Grid::arrange( const Rect& rect, const UIContext& ctx )
{
	rect_ = rect;

	const float_t spacing = get_spacing( ctx );
	const float_t cell_w = ( rect.w - spacing * static_cast< float_t >( columns_ - 1 ) ) / static_cast< float_t >( columns_ );
	const size_t n = children_.size();

	row_heights_.clear();
	float_t y = rect.y;

	for ( size_t start = 0; start < n; start += static_cast< size_t >( columns_ ) )
	{
		const size_t end = std::min( n, start + static_cast< size_t >( columns_ ) );
		float_t row_h = 0.f;

		for ( size_t i = start; i < end; i++ )
		{
			if ( children_[ i ]->is_visible() )
			{
				row_h = std::max( row_h, children_[ i ]->measure_height( cell_w, ctx ) );
			}
		}

		for ( size_t i = start; i < end; i++ )
		{
			const float_t x = rect.x + ( cell_w + spacing ) * static_cast< float_t >( i - start );
			children_[ i ]->arrange( Rect( x, y, cell_w, row_h ), ctx );
		}

		row_heights_.push_back( row_h );
		y += row_h + spacing;
	}
}

int Grid::nav_next_index( int index, NavDir dir ) const
{
	const int n = static_cast< int >( children_.size() );
	const int column = index % columns_;

	switch ( dir )
	{
		case NavDir::LEFT:
			return column > 0 ? index - 1 : -1;

		case NavDir::RIGHT:
			return ( column < columns_ - 1 && index + 1 < n ) ? index + 1 : -1;

		case NavDir::UP:
			return index - columns_ >= 0 ? index - columns_ : -1;

		case NavDir::DOWN:
		{
			if ( index + columns_ < n )
			{
				return index + columns_;
			}

			// 最後の行が途中までのときは、下の行の最後のセルへ
			const int next_row_start = ( index / columns_ + 1 ) * columns_;
			return next_row_start < n ? n - 1 : -1;
		}
	}

	return -1;
}

} // namespace core::ui
