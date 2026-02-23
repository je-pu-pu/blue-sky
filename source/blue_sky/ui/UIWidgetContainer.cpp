#include "UIWidgetContainer.h"
#include "UIWidget.h"
#include "UIRenderer.h"

#include <blue_sky/Input.h>

namespace blue_sky::ui
{

void UIWidgetContainer::update_focus( int new_index )
{
	if ( new_index < 0 || new_index >= static_cast< int >( widgets_.size() ) )
	{
		return;
	}

	if ( new_index == focused_index_ ) return;

	if ( focused_index_ >= 0 && focused_index_ < static_cast< int >( widgets_.size() ) )
	{
		widgets_[ focused_index_ ]->set_focused( false );
	}

	focused_index_ = new_index;
	widgets_[ focused_index_ ]->set_focused( true );
}

void UIWidgetContainer::layout()
{
	for ( int i = 0; i < static_cast< int >( widgets_.size() ); i++ )
	{
		float_t iy = y_ + ( item_height_ + spacing_ ) * static_cast< float_t >( i );
		widgets_[ i ]->set_position( x_, iy );
		widgets_[ i ]->set_size( width_, item_height_ );
	}

	// 初期フォーカス
	if ( ! widgets_.empty() )
	{
		widgets_[ focused_index_ ]->set_focused( true );
	}
}

void UIWidgetContainer::update( Input* input, UIRenderer& renderer )
{
	if ( widgets_.empty() ) return;

	// キーボード : 上下キーでフォーカス移動
	if ( input->push( Input::Button::UP ) )
	{
		int next = focused_index_ - 1;
		if ( next < 0 ) next = static_cast< int >( widgets_.size() ) - 1;
		update_focus( next );
	}

	if ( input->push( Input::Button::DOWN ) )
	{
		int next = focused_index_ + 1;
		if ( next >= static_cast< int >( widgets_.size() ) ) next = 0;
		update_focus( next );
	}

	// マウスホバーによるフォーカス切替（マウスが移動した場合のみ）
	{
		int mx = input->get_mouse_x();
		int my = input->get_mouse_y();

		bool is_mouse_moved = ( mx != last_mouse_x_ || my != last_mouse_y_ );
		last_mouse_x_ = mx;
		last_mouse_y_ = my;

		if ( is_mouse_moved )
		{
			float_t fmx = renderer.physical_to_virtual_x( static_cast< float_t >( mx ) );
			float_t fmy = renderer.physical_to_virtual_y( static_cast< float_t >( my ) );

			for ( int i = 0; i < static_cast< int >( widgets_.size() ); i++ )
			{
				if ( widgets_[ i ]->hit_test( fmx, fmy ) )
				{
					update_focus( i );
					break;
				}
			}
		}
	}

	// フォーカス中のウィジェットに入力を転送
	if ( focused_index_ >= 0 && focused_index_ < static_cast< int >( widgets_.size() ) )
	{
		widgets_[ focused_index_ ]->update( input, renderer );
	}
}

void UIWidgetContainer::render( UIRenderer& renderer )
{
	for ( auto* widget : widgets_ )
	{
		widget->render( renderer );
	}
}

UIWidget* UIWidgetContainer::get_focused_widget() const
{
	if ( focused_index_ >= 0 && focused_index_ < static_cast< int >( widgets_.size() ) )
	{
		return widgets_[ focused_index_ ];
	}
	return nullptr;
}

} // namespace blue_sky::ui
