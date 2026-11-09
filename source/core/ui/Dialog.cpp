#include "Dialog.h"
#include "Button.h"
#include "Label.h"
#include "Renderer.h"
#include "Stack.h"
#include "Theme.h"

#include <algorithm>

namespace core::ui
{

void show_dialog( Screen& screen, DialogSpec spec )
{
	auto dialog = std::make_unique< Dialog >( std::move( spec ) );
	Dialog* raw = dialog.get();
	raw->set_screen( & screen );

	screen.push_layer( std::move( dialog ), [raw] { raw->press_cancel(); }, true );
}

Dialog::Dialog( DialogSpec spec )
	: spec_( std::move( spec ) )
	, remaining_( spec_.timeout_sec )
{
	message_ = add< Label >( spec_.message );
	message_->set_wrap( true );
	message_->set_align( core::graphics::HAlign::CENTER );

	if ( spec_.message_provider )
	{
		message_->set_text_provider( spec_.message_provider );
	}

	auto* row = add< Stack >( Axis::HORIZONTAL );

	for ( size_t i = 0; i < spec_.buttons.size(); i++ )
	{
		const int index = static_cast< int >( i );
		buttons_.push_back( row->add< Button >( spec_.buttons[ i ].text_key, [this, index] { press( index ); } ) );
	}
}

void Dialog::press( int index )
{
	if ( closed_ || index < 0 || index >= static_cast< int >( spec_.buttons.size() ) )
	{
		return;
	}

	closed_ = true;

	if ( ! screen_ )
	{
		return;
	}

	// コールバックは値でコピーして post（ダイアログが破棄された後に実行されても安全）
	auto callback = spec_.buttons[ index ].on_press;

	if ( callback )
	{
		screen_->post( callback );
	}

	screen_->close_layer( this );
}

void Dialog::press_cancel()
{
	if ( spec_.cancel_index >= 0 )
	{
		press( spec_.cancel_index );
	}
}

Widget* Dialog::get_default_button() const
{
	if ( spec_.default_index >= 0 && spec_.default_index < static_cast< int >( buttons_.size() ) )
	{
		return buttons_[ spec_.default_index ];
	}

	return buttons_.empty() ? nullptr : buttons_.front();
}

float_t Dialog::measure_height( float_t width, const UIContext& ctx ) const
{
	const auto& t = ctx.theme;
	const float_t inner_w = width - t.padding * 2.f;

	return t.padding * 2.f + message_->measure_height( inner_w, ctx ) + t.padding + t.row_height;
}

void Dialog::arrange( const Rect& screen_rect, const UIContext& ctx )
{
	const auto& t = ctx.theme;
	const float_t w = std::min( t.dialog_width, screen_rect.w );
	const float_t h = measure_height( w, ctx );

	rect_ = Rect( screen_rect.x + ( screen_rect.w - w ) * 0.5f, screen_rect.y + ( screen_rect.h - h ) * 0.5f, w, h );

	const Rect inner = rect_.inset( t.padding );
	const float_t message_h = message_->measure_height( inner.w, ctx );

	message_->arrange( Rect( inner.x, inner.y, inner.w, message_h ), ctx );
	child_at( 1 )->arrange( Rect( inner.x, inner.bottom() - t.row_height, inner.w, t.row_height ), ctx );
}

void Dialog::update( float_t dt, UIContext& ctx )
{
	Widget::update( dt, ctx );

	if ( spec_.timeout_sec > 0.f && ! closed_ )
	{
		remaining_ -= dt;

		if ( remaining_ <= 0.f )
		{
			remaining_ = 0.f;
			press_cancel();
		}
	}

	// メッセージの高さが変わるかもしれないので、文字列を毎フレーム作る場合はレイアウトし直す
	if ( spec_.message_provider )
	{
		ctx.screen.invalidate_layout();
	}
}

void Dialog::render( Renderer& renderer, const UIContext& ctx ) const
{
	draw_brush( renderer, rect_, ctx.theme.panel );
	draw_brush( renderer, Rect( rect_.x, rect_.y, rect_.w, 6.f ), pick_brush( renderer, ctx.theme.dialog_header, ctx.theme.accent ) );
	render_children( renderer, ctx );
}

} // namespace core::ui
