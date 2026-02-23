#include "VerticalMenu.h"
#include "Renderer.h"

#include <blue_sky/Input.h>

namespace blue_sky::ui
{

VerticalMenu::VerticalMenu()
	: normal_color_( 0.8f, 0.8f, 0.8f, 1.f )
	, selected_color_( 1.f, 1.f, 0.5f, 1.f )
{
}

void VerticalMenu::add_item( const string_t& text, std::function< void() > on_select )
{
	items_.push_back( { text, on_select } );
}

void VerticalMenu::set_item_text( int index, const string_t& text )
{
	if ( index >= 0 && index < static_cast< int >( items_.size() ) )
	{
		items_[ index ].text = text;
	}
}

void VerticalMenu::clear_items()
{
	items_.clear();
	selected_index_ = 0;
}

void VerticalMenu::set_position( float_t x, float_t y )
{
	x_ = x;
	y_ = y;
}

void VerticalMenu::set_width( float_t w )
{
	width_ = w;
}

void VerticalMenu::set_item_height( float_t h )
{
	item_height_ = h;
}

void VerticalMenu::set_colors( const Color& normal, const Color& selected )
{
	normal_color_ = normal;
	selected_color_ = selected;
}

void VerticalMenu::center_on_screen( const Renderer& renderer )
{
	float_t total_height = item_height_ * static_cast< float_t >( items_.size() );

	x_ = ( renderer.get_screen_width() - width_ ) * 0.5f;
	y_ = ( renderer.get_screen_height() - total_height ) * 0.5f;
}

void VerticalMenu::update( Input* input, const Renderer& renderer )
{
	if ( items_.empty() )
	{
		return;
	}

	// キーボード / コントローラー : 上下キーで選択
	if ( input->push( Input::Button::UP ) )
	{
		select_prev();
	}

	if ( input->push( Input::Button::DOWN ) )
	{
		select_next();
	}

	// マウスホバーによる項目選択（マウスが移動した場合のみ）
	{
		int mx = input->get_mouse_x();
		int my = input->get_mouse_y();

		bool is_mouse_moved = ( mx != last_mouse_x_ || my != last_mouse_y_ );
		last_mouse_x_ = mx;
		last_mouse_y_ = my;

		if ( is_mouse_moved )
		{
			// 物理ピクセル座標 → 仮想座標に変換
			float_t fmx = renderer.physical_to_virtual_x( static_cast< float_t >( mx ) );
			float_t fmy = renderer.physical_to_virtual_y( static_cast< float_t >( my ) );

			for ( int i = 0; i < static_cast< int >( items_.size() ); i++ )
			{
				float_t item_y = y_ + item_height_ * static_cast< float_t >( i );

				if ( fmx >= x_ && fmx <= x_ + width_ && fmy >= item_y && fmy < item_y + item_height_ )
				{
					selected_index_ = i;
					break;
				}
			}
		}
	}

	// 決定 : Enter / マウス左クリック / コントローラーボタン
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

void VerticalMenu::render( Renderer& renderer )
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

void VerticalMenu::set_selected_index( int index )
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

void VerticalMenu::select_next()
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

void VerticalMenu::select_prev()
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
