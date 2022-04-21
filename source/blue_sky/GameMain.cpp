#include "GameMain.h"
#include "App.h"

/// @todo SceneManager などでやる
#include <Scene/TitleScene.h>
#include <Scene/StageSelectScene.h>
#include <Scene/StoryTextScene.h>
#include <Scene/GamePlayScene.h>
#include <Scene/EndingScene.h>

#include <Scene/CanvasTestScene.h>
#include <Scene/DebugScene.h>
#include <Scene/ParticleSystemTestScene.h>
#include <Scene/TransformTestScene.h>

#include "ActiveObjectManager.h"

// #include "ConstantBuffer.h"

#include "Input.h"

#include "ActiveObjectPhysics.h"

#include "ScriptManager.h"

#include <blue_sky/graphics/Direct3D11/GraphicsManager.h>

#include <core/graphics/Direct3D11/Direct3D11.h>
#include <core/graphics/Direct3D11/BulletDebugDraw.h>
#include <core/graphics/Direct3D11/Effect.h>
#include <core/graphics/Shader.h>
#include <core/graphics/OculusRift.h>

#include <core/sound/SoundManager.h>
#include <core/sound/Sound.h>

#include <core/input/DirectInput/DirectInput.h>

#include <win/Version.h>
#include <win/Clipboard.h>

#include <game/Config.h>
#include <game/MainLoop.h>

#include <common/math.h>
#include <common/log.h>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/operations.hpp>

#include <sstream>

namespace blue_sky
{

//■コンストラクタ
GameMain::GameMain()
	: total_elapsed_time_( 0.f )
	, is_display_fps_( false )
	, is_command_mode_( false )
{
	// common::log( "log/debug.log", "init" );

	boost::filesystem::create_directory( "log" );

	win::Version version;
	version.log( "log/windows_version.log" );

	// get_app()->clip_cursor( true );

	save_data_ = new Config();
	save_data_->load_file( "save/blue-sky.save" );

	// Direct3D
	direct_3d_ = new Direct3D(
		get_app()->GetWindowHandle(),
		get_app()->get_width(),
		get_app()->get_height(),
		get_app()->is_full_screen(),
		get_config()->get( "graphics.multisample.count", 4 ), 
		get_config()->get( "graphics.multisample.quality", 2 )
	);
	direct_3d_->get_effect()->load( "media/shader/main.fx" );
	direct_3d_->create_default_input_layout();

	if ( get_config()->get( "graphics.font_enabled", 1 ) )
	{
		direct_3d_->setup_font();
	}

	ImGui::CreateContext();
	ImGui_ImplWin32_Init( get_app()->GetWindowHandle() );
	ImGui_ImplDX11_Init( direct_3d_->getDevice(), direct_3d_->getImmediateContext() );


	direct_input_ = new DirectInput( get_app()->GetInstanceHandle(), get_app()->GetWindowHandle() );

	input_ = new Input();
	input_->set_direct_input( direct_input_.get() );
	input_->load_config( * get_config() );

	// Oculus Rift
	if ( get_config()->get( "input.oculus_rift.enabled", 0 ) )
	{
		oculus_rift_ = new OculusRift( direct_3d_.get() );
	}

	bullet_debug_draw_ = new core::graphics::direct_3d_11::BulletDebugDraw( direct_3d_.get() );
	bullet_debug_draw_->setDebugMode( get_config()->get< int >( "graphics.debug_bullet", 0 ) );

	physics_manager_ = new ActiveObjectPhysics();
	physics_manager_->setDebugDrawer( bullet_debug_draw_.get() );

	graphics_manager_ = new blue_sky::graphics::direct_3d_11::GraphicsManager( direct_3d_.get() );
	graphics_manager_->set_debug_axis_enabled( get_config()->get< int >( "graphics.debug_axis", 0 ) );
	
	sound_manager_ = new SoundManager( get_app()->GetWindowHandle() );
	sound_manager_->set_mute( get_config()->get( "audio.mute", 0 ) != 0 );
	sound_manager_->set_volume( get_config()->get( "audio.volume", 1.f ) );
	
	sound_manager_->load( "ok" );
	sound_manager_->load( "cancel" );
	sound_manager_->load( "click" );

	script_manager_ = new ScriptManager();
	script_manager_->load_command_history( "log/script.log" );
	setup_script_command();

	active_object_manager_ = new ActiveObjectManager();

	// MainLoop
	main_loop_ = new MainLoop( 60 );

	is_display_fps_ = get_config()->get( "graphics.display_fps", 0 ) != 0;

	/// @todo SceneManager でやる
	scene_creator_map_[ "title"        ] = [this] () { return new TitleScene( this ); };
	scene_creator_map_[ "stage_select" ] = [this] () { return new StageSelectScene( this ); };
	scene_creator_map_[ "stage_intro"  ] = [this] () { return new StoryTextScene( this, ( std::string( "media/stage/" ) + get_stage_name() + ".intro" ).c_str(), "game_play" );	};
	scene_creator_map_[ "game_play"    ] = [this] () { return new GamePlayScene( this ); };
	scene_creator_map_[ "stage_outro"  ] = [this] () { return new StoryTextScene( this, ( std::string( "media/stage/" ) + get_stage_name() + ".outro" ).c_str(), "stage_select" ); };
	scene_creator_map_[ "ending"       ] = [this] () { return new EndingScene( this ); };
	
	scene_creator_map_[ "debug"        ] = [this] () { return new DebugScene( this ); };
	scene_creator_map_[ "canvas_test"  ] = [this] () { return new CanvasTestScene( this ); };
	scene_creator_map_[ "particle_system_test"	] = [this]() { return new ParticleSystemTestScene( this ); };
	scene_creator_map_[ "transform_test"		] = [this]() { return new TransformTestScene( this ); };

	update_render_data_for_game();
}

//■デストラクタ
GameMain::~GameMain()
{
	ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

	get_config()->set< int >( "audio.mute", sound_manager_->is_mute() );
	get_config()->set< int >( "graphics.full_screen", get_direct_3d()->is_full_screen() );

	get_config()->save_file( "blue-sky.config" );

	scene_.release();

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
	get_script_manager()->set_function( "debug_axis", [this] ( int on ) { get_graphics_manager()->set_debug_axis_enabled( on ); } );
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
ActiveObject* GameMain::create_object( const char_t* class_name )
{
	ActiveObject* active_object = get_active_object_manager()->create_object( class_name );

	/// @todo 例外にしてスクリプト呼び出し側でキャッチできるようにする
	if ( ! active_object )
	{
		return 0;
	}

	active_object->set_rigid_body( get_physics_manager()->add_active_object_as_box( active_object ) );
	active_object->set_model( get_graphics_manager()->load_model( class_name ) );

	/// @todo 同じ種類の複数の ActiveObject が、Shader を共有せず、個別に保持できるようにする

	return active_object;
}

ActiveObject* GameMain::clone_object( const ActiveObject* o )
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

	total_elapsed_time_ += main_loop_->get_elapsed_sec();

	/// @todo 別スレッド化
	get_sound_manager()->update();
	
	if ( get_app()->is_active() && ! is_command_mode_ )
	{
		direct_input_->update();
		input_->update();
	}
	else
	{
		input_->update_null();
	}

	if ( oculus_rift_ )
	{
		oculus_rift_->update();
	}

	if ( input_->push( Input::Button::ESCAPE ) )
	{
		if ( scene_->get_name() == "title" )
		{
			get_app()->close();
		}
		else if ( scene_->get_name() == "game_play" )
		{
			scene_->set_next_scene( "stage_select" );
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
		}

		game::Sound* cancel = sound_manager_->get_sound( "cancel" );

		if ( cancel )
		{
			cancel->play( false );
		}
	}

	ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

	scene_->update();

	render();

	check_scene_transition();

	return true;
}

void GameMain::render()
{
	scene_->render();

	if ( is_command_mode_ )
	{
		graphics_manager_->draw_text_at_center( ( "> " + user_command_ ).c_str(), Color::White );
		graphics_manager_->draw_text( 0, get_height() / 2.f, static_cast< float_t >( get_width() ), static_cast< float_t >( get_height() ), get_script_manager()->get_output().c_str(), Color::White );

		/// @todo OculusRift にも文字を描画できるようにする
		/*
		if ( get_oculus_rift() )
		{
			get_oculus_rift()->setup_rendering();
			get_oculus_rift()->setup_rendering_for_left_eye();
			
			graphics_manager_->draw_text_at_center( ( "> " + user_command_ ).c_str(), Color::White );

			get_oculus_rift()->setup_rendering_for_right_eye();

			graphics_manager_->draw_text_at_center( ( "> " + user_command_ ).c_str(), Color::White );
		}
		*/
	}

	ImGui::Render();
	get_direct_3d()->set_default_render_target( false );
    ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData() );

	direct_3d_->present();
}

void GameMain::on_key_down( char_t key )
{
	if ( is_command_mode_ )
	{
		edit_command( key );
	}
	else
	{
		if ( key == '\r' )
		{
			is_command_mode_ = true;
		}
	}

	get_app()->show_cursor( is_command_mode_ );
}

void GameMain::edit_command( char_t key )
{
	if ( key == '\r' )
	{
		boost::trim( user_command_ );

		if ( user_command_ == "" )
		{
			is_command_mode_ = false;
		}
		else
		{
			try
			{
				get_script_manager()->exec( user_command_, true );
			}
			catch ( const ScriptError& e )
			{
				get_app()->show_error_message( e.what() );
			}

			user_command_ = "";
		}
	}
	else if ( key == '\t' )
	{
		get_script_manager()->auto_complete( user_command_ );
	}
	else if ( key == '\b' )
	{
		if ( user_command_.size() > 0 )
		{
			user_command_.resize( user_command_.size() - 1 );
		}
	}
	else if ( key == 0x16 )
	{
		user_command_ += win::Clipboard::get_text();
	}
	else
	{
		user_command_ += key;
	}
}

void GameMain::on_special_key_down( int key )
{
	if ( key == KEY_UP )
	{
		if ( is_command_mode_ )
		{
			user_command_ = get_script_manager()->get_prev_hisotry_command();
		}
	}
	else if ( key == KEY_DOWN )
	{
		if ( is_command_mode_ )
		{
			user_command_ = get_script_manager()->get_next_hisotry_command();
		}
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
		get_direct_3d()->switch_full_screen();
		get_app()->set_full_screen( get_direct_3d()->is_full_screen() );
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
		setup_scene( "debug" );
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
	auto i = scene_creator_map_.find( scene_name );

	if ( i == scene_creator_map_.end() )
	{
		// COMMON_THROW_EXCEPTION_MESSAGE( std::string( "worng next_scene : " ) + scene_name );
		return;
	}

	scene_.release();

	sound_manager_->pop_group();
	sound_manager_->push_group( scene_name.c_str() );
	
	scene_ = i->second();

	scene_->set_name( scene_name );
	scene_->set_next_stage_name( get_stage_name() );

	get_app()->clip_cursor( scene_->is_clip_cursor_required() );
}

float_t GameMain::get_elapsed_time() const
{
	return get_main_loop()->get_elapsed_sec();
}

} // namespace blue_sky