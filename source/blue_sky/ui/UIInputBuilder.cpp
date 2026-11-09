#include "UIInputBuilder.h"
#include "Renderer.h"

#include <blue_sky/Input.h>

#include <windows.h>

namespace blue_sky::ui
{

using core::ui::InputDevice;
using core::ui::UIInput;
using Button = core::input::Button;

core::ui::UIInput UIInputBuilder::build( Input& input, const Renderer& renderer, float_t dt )
{
	UIInput in;

	// 方向（矢印キー / パッド）＋キーリピート
	const bool down_up    = input.is_key_down( VK_UP )    || input.is_pad_dir_down( Button::UP );
	const bool down_down  = input.is_key_down( VK_DOWN )  || input.is_pad_dir_down( Button::DOWN );
	const bool down_left  = input.is_key_down( VK_LEFT )  || input.is_pad_dir_down( Button::LEFT );
	const bool down_right = input.is_key_down( VK_RIGHT ) || input.is_pad_dir_down( Button::RIGHT );

	in.up    = repeat_[ 0 ].update( down_up, dt );
	in.down  = repeat_[ 1 ].update( down_down, dt );
	in.left  = repeat_[ 2 ].update( down_left, dt );
	in.right = repeat_[ 3 ].update( down_right, dt );

	// 決定・キャンセル・タブ
	const DWORD pad_a = input.get_joystick_button_mask( Button::A );
	const DWORD pad_b = input.get_joystick_button_mask( Button::B );
	const DWORD pad_l = input.get_joystick_button_mask( Button::L );
	const DWORD pad_r = input.get_joystick_button_mask( Button::R );

	in.confirm  = input.is_key_pushed( VK_RETURN ) || input.is_pad_mask_pushed( pad_a );
	in.cancel   = input.is_key_pushed( VK_ESCAPE ) || input.is_pad_mask_pushed( pad_b );
	in.tab_prev = input.is_key_pushed( 'Q' ) || input.is_key_pushed( VK_PRIOR ) || input.is_pad_mask_pushed( pad_l );
	in.tab_next = input.is_key_pushed( 'E' ) || input.is_key_pushed( VK_NEXT )  || input.is_pad_mask_pushed( pad_r );

	// ポインタ（クライアント座標 → 仮想座標）
	in.pointer_x = renderer.physical_to_virtual_x( static_cast< float_t >( input.get_mouse_x() ) );
	in.pointer_y = renderer.physical_to_virtual_y( static_cast< float_t >( input.get_mouse_y() ) );
	in.pointer_moved = in.pointer_x != last_mouse_x_ || in.pointer_y != last_mouse_y_;
	in.pointer_pressed = input.is_key_pushed( VK_LBUTTON );
	in.pointer_down = input.is_key_down( VK_LBUTTON );
	in.pointer_released = input.is_key_released( VK_LBUTTON );
	in.wheel = input.pop_mouse_wheel_queue();

	last_mouse_x_ = in.pointer_x;
	last_mouse_y_ = in.pointer_y;

	in.pushed_key = input.get_pushed_bindable_key();

	// 文字（WM_CHAR で届いたもの。TextField が使う）
	in.text = input.take_text();

	// 独自の操作
	for ( const auto& b : bindings_ )
	{
		bool pushed = input.is_pad_mask_pushed( b.pad_mask );

		for ( int key : b.keys )
		{
			pushed = pushed || input.is_key_pushed( key );
		}

		if ( pushed )
		{
			in.actions.push_back( b.action );
		}
	}

	// 最後に使ったデバイス
	if ( input.is_pad_active() )
	{
		last_device_ = InputDevice::GAMEPAD;
	}
	else if ( in.pointer_moved || in.pointer_pressed || in.wheel != 0 || in.pushed_key != 0 || in.confirm || in.cancel || down_up || down_down || down_left || down_right )
	{
		last_device_ = InputDevice::KEYBOARD_MOUSE;
	}

	in.last_device = last_device_;

	return in;
}

void UIInputBuilder::bind_action( core::ui::ActionId action, std::vector< int > keys, DWORD pad_mask )
{
	bindings_.push_back( ActionBinding{ action, std::move( keys ), pad_mask } );
}

void UIInputBuilder::reset( Input& input, const Renderer& renderer )
{
	// 画面を開く前に打たれた文字は捨てる
	input.take_text();

	for ( auto& r : repeat_ )
	{
		r.reset();
	}

	last_mouse_x_ = renderer.physical_to_virtual_x( static_cast< float_t >( input.get_mouse_x() ) );
	last_mouse_y_ = renderer.physical_to_virtual_y( static_cast< float_t >( input.get_mouse_y() ) );
}

} // namespace blue_sky::ui
