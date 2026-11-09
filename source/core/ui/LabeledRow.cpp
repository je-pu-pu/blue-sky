#include "LabeledRow.h"
#include "Renderer.h"
#include "Theme.h"

namespace core::ui
{

float_t LabeledRow::measure_height( float_t, const UIContext& ctx ) const
{
	return ctx.theme.row_height;
}

void LabeledRow::arrange( const Rect& rect, const UIContext& ctx )
{
	rect_ = rect;

	const Rect inner = rect.inset( ctx.theme.row_padding_x, 0.f );
	const float_t label_w = inner.w * ctx.theme.label_ratio;

	label_rect_ = Rect( inner.x, inner.y, label_w, inner.h );
	value_rect_ = Rect( inner.x + label_w, inner.y, inner.w - label_w, inner.h );
}

Color LabeledRow::value_color( const UIContext& ctx ) const
{
	if ( ! is_enabled() ) return ctx.theme.text_disabled;
	return focused_ ? ctx.theme.text_focused : ctx.theme.text;
}

void LabeledRow::render_row( Renderer& renderer, const UIContext& ctx ) const
{
	const auto& t = ctx.theme;

	draw_brush( renderer, rect_, focused_ ? t.row_focused : t.row );
	renderer.draw_text( label_rect_, ctx.tr( label_key_ ).c_str(), make_text_style( value_color( ctx ), t.font_size_normal ) );
}

} // namespace core::ui
