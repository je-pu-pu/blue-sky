#include "UIVerticalMenu.h"
#include "UIRenderer.h"

#include <blue_sky/Input.h>

namespace blue_sky::ui
{

UIVerticalMenu::UIVerticalMenu()
	: normal_color_( 0.8f, 0.8f, 0.8f, 1.f )
	, selected_color_( 1.f, 1.f, 0.5f, 1.f )
{
}

void UIVerticalMenu::add_item( const string_t& text, std::function< void() > on_select )
{
	items_.push_back( { text, on_select } );
}

void UIVerticalMenu::set_item_text( int index, const string_t& text )
{
	if ( index >= 0 && index < static_cast< int >( items_.size() ) )
	{
		items_[ index ].text = text;
	}
}

void UIVerticalMenu::clear_items()
{
	items_.clear();
	selected_index_ = 0;
}

void UIVerticalMenu::set_position( float_t x, float_t y )
{
	x_ = x;
	y_ = y;
}

void UIVerticalMenu::set_width( float_t w )
{
	width_ = w;
}

void UIVerticalMenu::set_item_height( float_t h )
{
	item_height_ = h;
}

void UIVerticalMenu::set_colors( const Color& normal, const Color& selected )
{
	normal_color_ = normal;
	selected_color_ = selected;
}

void UIVerticalMenu::center_on_screen( const UIRenderer& renderer )
{
	float_t total_height = item_height_ * static_cast< float_t >( items_.size() );

	x_ = ( static_cast< float_t >( renderer.get_screen_width() ) - width_ ) * 0.5f;
	y_ = ( static_cast< float_t >( renderer.get_screen_height() ) - total_height ) * 0.5f;
}

void UIVerticalMenu::update( Input* input )
{
	if ( items_.empty() )
	{
		return;
	}

	if ( input->push( Input::Button::UP ) )
	{
		select_prev();
	}

	if ( input->push( Input::Button::DOWN ) )
	{
		select_next();
	}

	if ( input->push( Input::Button::A ) )
	{
		if ( selected_index_ >= 0 && selected_index_ < static_cast< int >( items_.size() ) )
		{
			if ( items_[ selected_index_ ].on_select )
			{
				items_[ selected_index_ ].on_select();
			}
		}
	}
}

void UIVerticalMenu::render( UIRenderer& renderer )
{
	for ( int i = 0; i < static_cast< int >( items_.size() ); i++ )
	{
		float_t item_y = y_ + item_height_ * static_cast< float_t >( i );
		bool is_selected = ( i == selected_index_ );

		const Color& color = is_selected ? selected_color_ : normal_color_;

		string_t display_text = is_selected ? ( cursor_text_ + items_[ i ].text ) : ( string_t( "  " ) + items_[ i ].text );

		renderer.draw_text( x_, item_y, width_, item_height_, display_text.c_str(), color );
	}
}

void UIVerticalMenu::set_selected_index( int index )
{
	if ( items_.empty() )
	{
		selected_index_ = 0;
		return;
	}

	selected_index_ = index;

	if ( selected_index_ < 0 )
	{
		selected_index_ = 0;
	}

	if ( selected_index_ >= static_cast< int >( items_.size() ) )
	{
		selected_index_ = static_cast< int >( items_.size() ) - 1;
	}
}

void UIVerticalMenu::select_next()
{
	if ( items_.empty() )
	{
		return;
	}

	selected_index_++;

	if ( selected_index_ >= static_cast< int >( items_.size() ) )
	{
		selected_index_ = 0;
	}
}

void UIVerticalMenu::select_prev()
{
	if ( items_.empty() )
	{
		return;
	}

	selected_index_--;

	if ( selected_index_ < 0 )
	{
		selected_index_ = static_cast< int >( items_.size() ) - 1;
	}
}

} // namespace blue_sky::ui
