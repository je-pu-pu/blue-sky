#include "UIOverlayScene.h"

#include <blue_sky/GameMain.h>
#include <blue_sky/Input.h>

#include <core/setting/SettingStore.h>
#include <core/ui/StringTable.h>
#include <core/ui/Theme.h>

#include <imgui.h>

namespace blue_sky
{

UIOverlayScene::UIOverlayScene()
	: renderer_( get_graphics_manager() )
	, screen_( GameMain::get_instance()->get_ui_theme(), GameMain::get_instance()->get_ui_strings(), renderer_ )
{
	screen_.set_sound_handler( [this] ( core::ui::UISoundEvent e )
	{
		switch ( e )
		{
			case core::ui::UISoundEvent::CONFIRM:	play_sound( "ok" ); break;
			case core::ui::UISoundEvent::CANCEL:	play_sound( "cancel" ); break;
			default:								play_sound( "click" ); break;
		}
	} );

	input_builder_.reset( * get_input(), renderer_ );
	screen_.suppress_input( 1 );

	// テーマの画像を先に読む（最初の描画やレイアウトの途中で読み込みが走って引っかからないように）
	core::ui::preload_theme_images( GameMain::get_instance()->get_ui_theme(), renderer_ );
}

UIOverlayScene::~UIOverlayScene() = default;

core::setting::SettingStore& UIOverlayScene::get_settings() const
{
	return * GameMain::get_instance()->get_settings();
}

void UIOverlayScene::update()
{
	Scene::update();

	const float_t dt = get_elapsed_time();
	core::ui::UIInput input = input_builder_.build( * get_input(), renderer_, dt );

	// 開発用のコンソール（ImGui）が入力を受け取っているときは、UI に渡さない
	// （以前はコンソールで打った Enter・Esc・Q/E が UI にも届いていた）
	const ImGuiIO& io = ImGui::GetIO();

	if ( io.WantCaptureKeyboard )
	{
		const core::ui::UIInput keep_pointer = input;
		input = core::ui::UIInput();
		input.pointer_x = keep_pointer.pointer_x;
		input.pointer_y = keep_pointer.pointer_y;
		input.pointer_moved = keep_pointer.pointer_moved;
		input.pointer_pressed = keep_pointer.pointer_pressed;
		input.pointer_down = keep_pointer.pointer_down;
		input.pointer_released = keep_pointer.pointer_released;
		input.wheel = keep_pointer.wheel;
		input.last_device = keep_pointer.last_device;
	}

	if ( io.WantCaptureMouse )
	{
		input.pointer_moved = input.pointer_pressed = input.pointer_down = input.pointer_released = false;
		input.wheel = 0;
	}

	screen_.update( input, dt );
}

void UIOverlayScene::render()
{
	screen_.render( renderer_ );
}

void UIOverlayScene::on_resume()
{
	input_builder_.reset( * get_input(), renderer_ );
	screen_.suppress_input( 1 );
}

} // namespace blue_sky
