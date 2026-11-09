#include "AnchorBox.h"

#include <algorithm>

namespace core::ui
{

Rect AnchorBox::place( const Rect& parent, float_t w, float_t h, Anchor anchor, float_t margin )
{
	const int column = static_cast< int >( anchor ) % 3;		// 0 = 左, 1 = 中央, 2 = 右
	const int row = static_cast< int >( anchor ) / 3;			// 0 = 上, 1 = 中央, 2 = 下

	w = std::min( w, std::max( 0.f, parent.w - margin * 2.f ) );
	h = std::min( h, std::max( 0.f, parent.h - margin * 2.f ) );

	const float_t x =
		column == 0 ? parent.x + margin :
		column == 1 ? parent.x + ( parent.w - w ) * 0.5f :
		              parent.right() - margin - w;

	const float_t y =
		row == 0 ? parent.y + margin :
		row == 1 ? parent.y + ( parent.h - h ) * 0.5f :
		           parent.bottom() - margin - h;

	return Rect( x, y, w, h );
}

float_t AnchorBox::measure_height( float_t width, const UIContext& ctx ) const
{
	if ( children_.empty() )
	{
		return 0.f;
	}

	const float_t w = width_ > 0.f ? std::min( width_, width ) : width;
	return children_.front()->measure_height( w, ctx ) + margin_ * 2.f;
}

void AnchorBox::arrange( const Rect& rect, const UIContext& ctx )
{
	rect_ = rect;

	if ( children_.empty() )
	{
		return;
	}

	Widget* child = children_.front().get();

	const float_t w = width_ > 0.f ? width_ : rect.w - margin_ * 2.f;
	const float_t h = child->measure_height( std::min( w, rect.w ), ctx );

	child->arrange( place( rect, w, h, anchor_, margin_ ), ctx );
}

} // namespace core::ui
