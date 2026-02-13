#include "GameMain.h"
#include "App.h"

#include "SceneManager.h"
#include "Scene/Scene.h"

#include "ActiveObjectManager.h"

// #include "ConstantBuffer.h"

#include "Input.h"

#include "ActiveObjectPhysics.h"

#include "ScriptManager.h"

#include <blue_sky/graphics/Direct3D11/GraphicsManager.h>

#include <core/ecs/EntityManager.h>
#include <core/Service.h>
#include <core/graphics/Direct3D11/Direct3D11.h>
#include <core/graphics/Direct3D11/BulletDebugDraw.h>
#include <core/graphics/Direct3D11/Effect.h>
#include <core/graphics/Shader.h>
#include <core/graphics/OculusRift.h>


#include <core/sound/DirectSound/SoundEngine.h>
#include <core/sound/PortAudio/SoundEngine.h>

#include <core/sound/SoundManager.h>
#include <core/sound/Sound.h>

#include <core/input/DirectInput/DirectInput.h>
#include <core/physics/PhysicsManager.h>

#include <core/Logger.h>

#include <win/Version.h>
#include <win/Clipboard.h>

#include <game/Config.h>
#include <game/MainLoop.h>

#include <common/math.h>
#include <common/log.h>

#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/operations.hpp>

namespace blue_sky
{

//■コンストラクタ
GameMain::GameMain()
	: is_display_fps_( false )
	, is_command_mode_( false )
	, is_show_cursor_( false )
	, user_command_( 1024, '\0' )
{
	// common::log( "log/debug.log", "init" );

	boost::filesystem::create_directory( "log" );

	win::Version version;
	version.log( "log/windows_version.log" );

	// get_app()->clip_cursor( true );

	save_data_.reset( new Config() );
	save_data_->load_file( "save/blue-sky.save" );

	// Direct3D
	direct_3d_.reset( new Direct3D(
		get_app()->GetWindowHandle(),
		get_app()->get_width(),
		get_app()->get_height(),
		get_app()->is_full_screen(),
		get_config()->get( "graphics.multisample.count", 4 ), 
		get_config()->get( "graphics.multisample.quality", 2 )
	) );

	direct_3d_->get_effect()->load( "media/shader/main.fx" );

	direct_3d_->create_default_input_layout();

	/// @todo 各シェーダーの effect_technique_ が古いままなのを解消する
	get_app()->watch_directory_change( "media/shader/", [this] () {
		direct_3d_->get_effect()->load( "media/shader/main.fx" );
		direct_3d_->create_default_input_layout();

		if ( graphics_manager_ )
		{
			graphics_manager_->refresh_all_shaders();
		}
	} );

	if ( get_config()->get( "graphics.font_enabled", 1 ) )
	{
		direct_3d_->setup_font();
	}

	ImGui::CreateContext();
	ImGui_ImplWin32_Init( get_app()->GetWindowHandle() );
	ImGui_ImplDX11_Init( direct_3d_->getDevice(), direct_3d_->getImmediateContext() );

	ImGuiIO& io = ImGui::GetIO();
	// io.Fonts->AddFontFromFileTTF( "media/font/uzura.ttf", 22 );
	io.Fonts->AddFontFromFileTTF( "media/font/rounded-mplus-1p-regular.ttf", 28 );


	direct_input_.reset( new DirectInput( get_app()->GetInstanceHandle(), get_app()->GetWindowHandle() ) );

	input_.reset( new Input() );
	input_->set_direct_input( direct_input_.get() );
	input_->load_config( * get_config() );

	// Oculus Rift
	if ( get_config()->get( "input.oculus_rift.enabled", 0 ) )
	{
		oculus_rift_.reset( new OculusRift( direct_3d_.get() ) );
	}

	bullet_debug_draw_.reset( new core::graphics::direct_3d_11::BulletDebugDraw( direct_3d_.get() ) );
	bullet_debug_draw_->setDebugMode( get_config()->get< int >( "graphics.debug_bullet", 0 ) );

	SceneManager::get_instance()->register_all_scene();

	active_object_physics_.reset( new ActiveObjectPhysics() );
	active_object_physics_->setDebugDrawer( bullet_debug_draw_.get() );

	physics_manager_.reset( new core::physics::PhysicsManager() );

	graphics_manager_.reset( new blue_sky::graphics::direct_3d_11::GraphicsManager( direct_3d_.get() ) );
	graphics_manager_->set_debug_axis_enabled( get_config()->get< int >( "graphics.debug_axis", 0 ) != 0 );
	
	// sound_engine_.reset( new core::sound::direct_sound::SoundEngine( get_app()->GetWindowHandle() ) );
	sound_engine_.reset( new core::sound::port_audio::SoundEngine() );
	sound_manager_.reset( new SoundManager( sound_engine_.get() ) );
	sound_manager_->set_mute( get_config()->get( "audio.mute", 0 ) != 0 );
	sound_manager_->set_volume( get_config()->get( "audio.volume", 1.f ) );
	
	sound_manager_->load( "ok" );
	sound_manager_->load( "cancel" );
	sound_manager_->load( "click" );

	// Service に登録
	core::Service::get_instance()->set_graphics_manager( graphics_manager_.get() );
	core::Service::get_instance()->set_sound_manager( sound_manager_.get() );
	core::Service::get_instance()->set_input_manager( input_.get() );
	core::Service::get_instance()->set_physics_manager( physics_manager_.get() );
	core::Service::get_instance()->set_time_manager( &time_manager_ );

	script_manager_.reset( new ScriptManager() );
	script_manager_->load_command_history( "log/script.log" );
	setup_script_command();

	active_object_manager_.reset( new ActiveObjectManager() );

	// MainLoop
	main_loop_.reset( new MainLoop( 60 ) );

	is_display_fps_ = get_config()->get( "graphics.display_fps", 0 ) != 0;

	update_render_data_for_game();
}

//■デストラクタ
GameMain::~GameMain()
{
	ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

	get_config()->set< int >( "audio.mute", sound_manager_->is_mute() );
	get_config()->set< int >( "graphics.full_screen", get_graphics_manager()->is_full_screen() );

	get_config()->save_file( "blue-sky.config" );

	get_script_manager()->save_command_history( "log/script.log" );
}

/**
 * ゲーム全体で共有可能なスクリプトコマンドを準備する
 *
 */
void GameMain::setup_script_command()
{
	get_script_manager()->exec( "function load( s ) dofile( 'media/script/' .. s ) end" );

	// Scene
	get_script_manager()->set_function( "scene", [this] ( const char_t* name ) { setup_scene( name ); } );
	get_script_manager()->set_function( "push_overlay", [this] ( const char_t* name ) { push_overlay_scene( name ); } );
	get_script_manager()->set_function( "pop_overlay", [this] () { pop_overlay_scene(); } );

	// Basic
	get_script_manager()->set_function( "color", [this] ( float_t r, float_t g, float_t b, float_t a ) { return Color( r, g, b, a ); } );
	get_script_manager()->set_function( "vector", [this] ( float_t x, float_t y, float_t z ) { return Vector( x, y, z ); } );

	// Fade
	get_script_manager()->set_function( "set_fade_color", [this] ( const Color& color ) { get_graphics_manager()->set_fade_color( color ); } );
	get_script_manager()->set_function( "start_fade_in", [this] ( float_t speed ) { get_graphics_manager()->start_fade_in( speed ); } );
	get_script_manager()->set_function( "start_fade_out", [this] ( float_t speed ) { get_graphics_manager()->start_fade_out( speed ); } );

	// ActiveObject
	get_script_manager()->set_function( "create_object", [this] ( const char_t* name ) { auto* o = create_object( name ); o->restart(); return o; } );
	get_script_manager()->set_function( "clone_object", [this] ( ActiveObject* o ) { auto* o2 = clone_object( o ); o2->restart(); return o2; } );
	get_script_manager()->set_function( "get_object", [this] ( const char_t* name ) { return get_active_object_manager()->get_active_object( name ); } );
	get_script_manager()->set_function( "set_name", [this] ( ActiveObject* o, const char_t* name ) { get_active_object_manager()->name_active_object( name, o ); } );
	get_script_manager()->set_function( "set_loc", [] ( ActiveObject* o, float_t x, float_t y, float_t z ) { o->set_location( x, y, z ); } );
	get_script_manager()->set_function( "set_dir", [] ( ActiveObject* o, float_t r ) { o->set_direction_degree( r ); } );
	get_script_manager()->set_function( "set_vel", [] ( ActiveObject* o, float_t x, float_t y, float_t z ) { o->set_velocity( Vector( x, y, z ) ); } );
	get_script_manager()->set_function( "set_start_loc", [] ( ActiveObject* o, float_t  x, float_t  y, float_t  z ) { o->set_start_location( x, y, z ); } );
	get_script_manager()->set_function( "set_start_rot", [] ( ActiveObject* o, float_t rx, float_t ry, float_t rz ) { o->set_start_rotation( rx, ry, rz ); } );
	get_script_manager()->set_function( "set_start_dir", [] ( ActiveObject* o, float_t r ) { o->set_start_direction_degree( r ); } );
	get_script_manager()->set_function( "set_mass", [] ( ActiveObject* o, float_t mass ) { o->set_mass( mass ); } );

	// GrahpicsManager
	get_script_manager()->set_function( "setup_shadow_map", [this] ( uint_t level = 3, uint_t size = 1024 ) { get_graphics_manager()->setup_shadow_map( level, size ); } );
	get_script_manager()->set_function( "set_sky_box", [this] ( const char_t* name ) { get_graphics_manager()->set_sky_box( name ); } );
	get_script_manager()->set_function( "unset_sky_box", [this] () { get_graphics_manager()->unset_sky_box(); } );
	get_script_manager()->set_function( "set_ground", [this] ( const char_t* name ) { get_graphics_manager()->set_ground( name ); } );
	get_script_manager()->set_function( "unset_ground", [this] () { get_graphics_manager()->unset_ground(); } );

	get_script_manager()->set_function( "set_paper_texture_type", [this] ( int_t type ) { get_graphics_manager()->set_paper_texture_type( type ); } );
	get_script_manager()->set_function( "set_ambient_color", [this] ( const Color& color ) { get_graphics_manager()->set_ambient_color( color ); } );
	get_script_manager()->set_function( "set_shadow_color", [this] ( const Color& color ) { get_graphics_manager()->set_shadow_color( color ); } );
	get_script_manager()->set_function( "set_shadow_paper_color", [this] ( const Color& color ) { get_graphics_manager()->set_shadow_paper_color( color ); } );
	get_script_manager()->set_function( "set_drawing_accent", [this] ( float_t accent ) { get_graphics_manager()->set_drawing_accent( accent ); } );
	get_script_manager()->set_function( "set_drawing_line_type", [this] ( int_t type ) { get_graphics_manager()->set_drawing_line_type( type ); } );

	get_script_manager()->set_function( "set_post_effect_shader", [this] ( core::graphics::Shader* s ) { get_graphics_manager()->set_post_effect_shader( s ); } );

	get_script_manager()->set_function( "load_model", [this] ( const char_t* name ) { return get_graphics_manager()->load_model( name ); } );
	get_script_manager()->set_function( "clone_model", [this] ( graphics::Model* m ) { auto* m2 = get_graphics_manager()->clone_model( m ); return m2; } );

	get_script_manager()->set_function( "get_shader", [this] ( const char_t* name ) { return get_graphics_manager()->get_shader( name ); } );
	get_script_manager()->set_function( "clone_shader", [this] ( const core::graphics::Shader* s ) { return get_graphics_manager()->clone_shader( s ); } );

	get_script_manager()->set_function( "load_texture", [this] ( const char_t* file_path ) { return get_graphics_manager()->load_texture( file_path ); } );
	// get_script_manager()->set_function( "load_named_texture", [this] ( const char_t* name, const char_t* file_path ) { return get_graphics_manager()->load_texture( name, file_path ); } );
	get_script_manager()->set_function( "get_texture", [this] ( const char_t* name ) { return get_graphics_manager()->get_texture( name ); } );

	// SoundManager
	get_script_manager()->set_function( "play_sound", [this] ( const char_t* name ) { if ( auto* s = get_sound_manager()->get_sound( name ) ) { s->play( false ); } } );

	// debug
	get_script_manager()->set_function( "debug_axis", [this] ( int on ) { get_graphics_manager()->set_debug_axis_enabled( on != 0 ); } );
	get_script_manager()->set_function( "debug_bullet", [this] ( int mode ) { bullet_debug_draw_->setDebugMode( mode ); } );

	get_script_manager()->set_function( "debug_print_resources", [this] () { get_graphics_manager()->debug_print_resources(); } );

	/// @todo ActiveObject に color を持たせる？
	// get_script_manager()->set_function( "set_object_object", [this] ( ActiveObject* o, const Color& c ) { o->set_co } );
}

/**
 * オブジェクトを生成する
 *
 * @param class_name  クラス名
 * @return 生成したオブジェクト
 * @todo 整理する。 loc, rot の指定をどうするか？ StaticObject の生成をどうするか？ Scenegraph に移動？
 */
ActiveObject* GameMain::create_object( const char_t* class_name ) const
{
	ActiveObject* active_object = get_active_object_manager()->create_object( class_name );

	/// @todo 例外にしてスクリプト呼び出し側でキャッチできるようにする
	if ( ! active_object )
	{
		return 0;
	}

	active_object->set_rigid_body( get_active_object_physics()->add_active_object_as_box( active_object ) );
	active_object->set_model( get_graphics_manager()->load_model( class_name ) );

	/// @todo 同じ種類の複数の ActiveObject が、Shader を共有せず、個別に保持できるようにする

	return active_object;
}

ActiveObject* GameMain::clone_object( const ActiveObject* o ) const
{
	return get_active_object_manager()->clone_object( o );
}

/**
 * ゲーム毎に更新する必要のある描画用の定数バッファを更新する
 *
 */
void GameMain::update_render_data_for_game() const
{
	GameConstantBufferData constant_buffer_data = {
		static_cast< float_t >( get_width() ),
		static_cast< float_t >( get_height() )
	};
		
	get_graphics_manager()->get_game_render_data()->update( & constant_buffer_data );
}

bool GameMain::update()
{
	if ( ! main_loop_->loop() )
	{
		return false;
	}

	time_manager_.update( main_loop_->get_elapsed_sec() );

	/// @todo 別スレッド化
	get_sound_manager()->update();
	
	if ( get_app()->is_active() )
	{
		direct_input_->update();
		input_->update();

		if ( is_command_mode_ )
		{
			input_->clear_mouse_move();
		}
	}
	else
	{
		input_->update_null();
	}

	if ( oculus_rift_ )
	{
		oculus_rift_->update();
	}

	// ESC キーはオーバーレイの有無に関わらず検出する (ブロックをバイパス)
	{
		const bool escape_blocked = input_->is_blocked();
		input_->set_blocked( false );
		const bool escape_pushed = input_->push( Input::Button::ESCAPE );
		input_->set_blocked( escape_blocked );

		if ( escape_pushed )
		{
			if ( has_overlay_scene() )
			{
				// オーバーレイが開いている場合は閉じる
				pop_overlay_scene();
				if ( auto* s = sound_manager_->get_sound( "cancel" ) ) { s->play( false ); }
			}
			else if ( scene_->get_name() == "title" )
			{
				get_app()->close();
			}
			else if ( scene_->get_name() == "game_play" )
			{
				// ゲームプレイ中はポーズメニューを開く
				push_overlay_scene( "pause_menu" );
				if ( auto* s = sound_manager_->get_sound( "ok" ) ) { s->play( false ); }
			}
			else
			{
				if (
					get_save_data()->get< int >( "stage.0-0", 0 ) > 0 &&
					get_save_data()->get< int >( "stage.0-1", 0 ) > 0 &&
					get_save_data()->get< int >( "stage.0-2", 0 ) > 0 )
				{
					scene_->set_next_scene( "title" );
				}
				else
				{
					get_app()->close();
				}

				if ( auto* s = sound_manager_->get_sound( "cancel" ) ) { s->play( false ); }
			}
		}
	}

	ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

	// オーバーレイシーンが存在する場合、ベースシーンの入力をブロック
	input_->set_blocked( has_overlay_scene() );

	if ( has_overlay_scene() )
	{
		// ベースシーンはポーズ中 (更新しない)
		// オーバーレイシーンのトップのみ更新
		overlay_scene_stack_.back()->update();
	}
	else
	{
		scene_->update();
	}

	render();

	// オーバーレイシーンの遷移チェック
	if ( has_overlay_scene() && ! overlay_scene_stack_.back()->get_next_scene().empty() )
	{
		string_t next = overlay_scene_stack_.back()->get_next_scene();

		if ( next == "pop" )
		{
			pop_overlay_scene();
		}
		else
		{
			pop_overlay_scene();
			push_overlay_scene( next );
		}
	}
	else
	{
		check_scene_transition();
	}

	return true;
}

void GameMain::render()
{
	// ベースシーンを常に描画
	scene_->render();

	// オーバーレイシーンを上から順に描画
	for ( auto& overlay : overlay_scene_stack_ )
	{
		overlay->render();
	}

	if ( is_command_mode_ )
    {
		render_console_window();
		render_scene_list_window();
	}

	ImGui::Render();
	get_graphics_manager()->set_default_render_target( false );
    ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData() );

	direct_3d_->present();
}

/**
 * @brief コンソールウィンドウを描画する
 */
void GameMain::render_console_window()
{
	if ( ! ImGui::Begin( "Console", 0 ) )
    {
		ImGui::End();
		return;
	}

	static auto scroll_to_bottom = false;

	const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
	if ( ImGui::BeginChild( "ScrollingRegion", ImVec2( 0, -footer_height_to_reserve ), 0, ImGuiWindowFlags_HorizontalScrollbar ) )
    {
		if ( ImGui::BeginPopupContextWindow() )
        {
            if ( ImGui::Selectable( "Clear" ) )
			{
				// clear log
			}

            ImGui::EndPopup();
        }

		for ( const auto& log: core::logger.get_log_list() )
		{
			auto has_color = false;

			switch ( log.get_type() )
			{
			case core::Logger::Log::Type::ERROR:
				ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 0.4f, 0.4f, 1.0f ) );
				has_color = true;
				break;
			case core::Logger::Log::Type::WARN:
				ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 0.7f, 0.1f, 1.0f ) );
				has_color = true;
				break;
			case core::Logger::Log::Type::INFO:
				break;
			case core::Logger::Log::Type::DEBUG:
				break;
			}
                    

			ImGui::TextUnformatted( log.get_message().c_str() );

			if ( has_color )
			{
				ImGui::PopStyleColor();
			}
		}

		if ( scroll_to_bottom )
		{
            ImGui::SetScrollHereY( 1.f );
		}

        scroll_to_bottom = false;
	}

	ImGui::EndChild();

	ImGui::Separator();

	const auto input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;

	ImGui::SetNextItemWidth( -FLT_MIN );

	bool reclaim_focus = false;

	const auto callback = [] (ImGuiInputTextCallbackData* data) -> int
	{
		auto game_main = static_cast< GameMain* >( data->UserData );
		string_t command;

		switch ( data->EventFlag )
		{
		case ImGuiInputTextFlags_CallbackCompletion:
			command = game_main->user_command_;

			game_main->get_script_manager()->auto_complete( command );

			if ( ! game_main->get_script_manager()->get_output().empty() )
			{
				core::logger.info( game_main->get_script_manager()->get_output() );
			}

			scroll_to_bottom = true;

			break;
		case ImGuiInputTextFlags_CallbackHistory:
			if ( data->EventKey == ImGuiKey_UpArrow )
			{
				command = game_main->get_script_manager()->get_prev_hisotry_command();
			
			}
			else if ( data->EventKey == ImGuiKey_DownArrow )
			{
				command = game_main->get_script_manager()->get_next_hisotry_command();	
			}
		}

		if ( command != game_main->user_command_ )
		{
			data->DeleteChars( 0, data->BufTextLen );
			data->InsertChars( 0, command.c_str() );
		}

		return 0;
	};

	if ( ImGui::InputText( "##command", & user_command_, input_text_flags, callback, this ) )
    {
		boost::trim( user_command_ );

		if ( user_command_ != "" )
		{
			core::logger.info( user_command_ );

			try
			{
				get_script_manager()->exec( user_command_, true );
			}
			catch ( const ScriptError& e )
			{
				// get_app()->show_error_message( e.what() );
				core::logger.error( e.what() );
			}

			user_command_ = "";
			scroll_to_bottom = true;
		}

		reclaim_focus = true;
	}

	ImGui::SetItemDefaultFocus();

    if ( reclaim_focus || ! ImGui::IsAnyItemActive() )
	{
		ImGui::SetKeyboardFocusHere( -1 );
	}

	ImGui::End();
}
/**
 * @brief シーン一覧ウィンドウを描画する
 */
void GameMain::render_scene_list_window()
{
	if ( ! ImGui::Begin( "SceneList" ) )
    {
		ImGui::End();
		return;
	}

	if ( ImGui::BeginListBox( "##SceneListBox", ImVec2( -FLT_MIN, 0 ) ) )
    {
		static int selected_item_index = -1;

		int n = 0;

        for ( const auto& scene: SceneManager::get_instance()->get_scene_generator_map() )
        {
            ImGuiSelectableFlags flags = n == selected_item_index ? ImGuiSelectableFlags_Highlight : 0;

            if ( ImGui::Selectable( scene.first.c_str(), n == selected_item_index, flags ) )
			{
				selected_item_index = n;
			}

			if ( ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked( 0 ) )
			{
				setup_scene( scene.first );
			}

			n++;
        }
        
		ImGui::EndListBox();
    }

	ImGui::End();
}

void GameMain::on_key_down( char_t )
{
	
}

void GameMain::on_special_key_down( int key )
{
	if ( key == KEY_F1 )
	{
		is_command_mode_ = ! is_command_mode_;
		set_show_cursor( is_command_mode_ );
		get_app()->clip_cursor( is_command_mode_ ? false : scene_->is_clip_cursor_required() );
	}
	else if ( key == KEY_F2 )
	{
		get_sound_manager()->set_mute( ! get_sound_manager()->is_mute() );
		game::Sound* bgm = get_sound_manager()->get_sound( "bgm" );

		if ( bgm )
		{
			bgm->play( true );
		}
	}
	else if ( key == KEY_F5 )
	{
		get_graphics_manager()->switch_full_screen();
		get_app()->set_full_screen( get_graphics_manager()->is_full_screen() );
	}

	/// @todo ちゃんとする
	scene_->on_function_key_down( key -  KEY_F1 + 1 );
}

void GameMain::on_mouse_wheel( int wheel )
{
	input_->push_mouse_wheel_queue( wheel > 0 ? 1 : -1 );
}

void GameMain::on_resize()
{
	if ( direct_3d_ )
	{
		direct_3d_->on_resize( get_app()->get_width(), get_app()->get_height() );
	}
}

/**
 * 次のシーンへの遷移をチェックする
 *
 */
void GameMain::check_scene_transition()
{
	if ( ! scene_->get_next_scene().empty() )
	{
		string_t next_scene = scene_->get_next_scene();

		set_stage_name( scene_->get_next_stage_name() );

		setup_scene( next_scene );

		save_data_->save_file( "save/blue-sky.save" );
	}
}

/**
 * シーンを準備する
 *
 */
void GameMain::setup_scene()
{
	if ( true )
	{
		// set_stage_name( "0-1" );
		// setup_scene( "game_play" );
		setup_scene( get_config()->get< string_t >( "debug.start_scene", "debug" ) );
		// setup_scene( "canvas_test" );

		return;
	}

	// Scene
	if ( get_save_data()->get< int >( "stage.0-2", 0 ) > 0 )
	{
		setup_scene( "title" );
	}
	else
	{
		if ( get_save_data()->get< int >( "stage.0-1", 0 ) > 0 )
		{
			set_stage_name( "0-2" );
		}
		else if ( get_save_data()->get< int >( "stage.0-0", 0 ) > 0 )
		{
			set_stage_name( "0-1" );
		}
		else
		{
			set_stage_name( "0-0" );
		}

		setup_scene( "stage_intro" );
	}
}

/**
 * シーンを準備する
 *
 * @param scene_name シーン名
 */
void GameMain::setup_scene( const string_t& scene_name )
{
	if ( ! SceneManager::get_instance()->is_scene_registered( scene_name ) )
	{
		// スクリプトの途中で例外が発生し、その後のスクリプトが実行されない問題に対処している？
		// COMMON_THROW_EXCEPTION_MESSAGE( std::string( "worng next_scene : " ) + scene_name );
		return;
	}

	sound_manager_->pop_group();
	sound_manager_->push_group( scene_name.c_str() );
	
	// 現在のシーンを解放
	scene_.reset();

	// シーンによって設定されていたデータをクリア
	{
		core::ecs::EntityManager::get_instance()->clear();
		get_graphics_manager()->set_main_camera_info( nullptr, nullptr );
	}

	// 新しいシーンを設定
	scene_.reset( SceneManager::get_instance()->generate_scene( scene_name ) );

	/// @todo 各 Scene クラスが自分の名前を持ち、
	scene_->set_name( scene_name );
	scene_->set_next_stage_name( get_stage_name() );

	get_app()->clip_cursor( scene_->is_clip_cursor_required() );
}

/**
 * オーバーレイシーンをスタックにプッシュする
 *
 * ベースシーンを破棄せずに、上にオーバーレイシーン (ポーズメニュー等) を重ねる。
 * ベースシーンの更新は停止し、入力はオーバーレイシーンのみが受け取る。
 *
 * @param scene_name シーン名
 */
void GameMain::push_overlay_scene( const string_t& scene_name )
{
	if ( ! SceneManager::get_instance()->is_scene_registered( scene_name ) )
	{
		return;
	}

	// ベースシーンをポーズ
	scene_->set_paused( true );

	// オーバーレイシーンを生成してスタックに追加
	std::unique_ptr< Scene > overlay( SceneManager::get_instance()->generate_scene( scene_name ) );
	overlay->set_name( scene_name );
	overlay->set_next_stage_name( get_stage_name() );

	overlay_scene_stack_.push_back( std::move( overlay ) );

	// カーソルを表示 (メニュー操作用)
	set_show_cursor( true );
	get_app()->clip_cursor( false );
}

/**
 * オーバーレイシーンをスタックからポップする
 *
 * スタックが空になった場合、ベースシーンのポーズを解除する。
 */
void GameMain::pop_overlay_scene()
{
	if ( overlay_scene_stack_.empty() )
	{
		return;
	}

	overlay_scene_stack_.pop_back();

	if ( overlay_scene_stack_.empty() )
	{
		// 全てのオーバーレイが閉じた → ベースシーンのポーズ解除
		scene_->set_paused( false );
		input_->set_blocked( false );

		set_show_cursor( is_command_mode_ );
		get_app()->clip_cursor( is_command_mode_ ? false : scene_->is_clip_cursor_required() );
	}
}

} // namespace blue_sky