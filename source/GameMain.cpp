#include "GameMain.h"
#include "App.h"

#include <Scene/CanvasTestScene.h>
#include <Scene/TitleScene.h>
#include <Scene/StageSelectScene.h>
#include <Scene/StoryTextScene.h>
#include <Scene/GamePlayScene.h>
#include <Scene/EndingScene.h>

#include "DrawingModelManager.h"
#include "ActiveObjectManager.h"

#include "ConstantBuffer.h"

#include "Input.h"

#include "OculusRift.h"

#include "ActiveObjectPhysics.h"

#include "SoundManager.h"
#include "Sound.h"

#include <core/graphics/Direct3D11/Direct3D11.h>
#include <core/graphics/Direct3D11/Direct3D11GraphicsManager.h>
#include <core/graphics/Direct3D11/Direct3D11BulletDebugDraw.h>
#include <core/graphics/Direct3D11/Direct3D11Effect.h>
#include <core/input/DirectInput/DirectInput.h>

#include <win/Version.h>

#include <game/Config.h>
#include <game/MainLoop.h>

#include <common/math.h>
#include <common/log.h>

#include <boost/filesystem/operations.hpp>

#include <sstream>


#pragma comment( lib, "game.lib" )
#pragma comment( lib, "win.lib" )

namespace blue_sky
{

//■コンストラクタ
GameMain::GameMain()
	: total_elapsed_time_( 0.f )
	, is_display_fps_( false )
{
	// common::log( "log/debug.log", "init" );

	boost::filesystem::create_directory( "log" );

	win::Version version;
	version.log( "log/windows_version.log" );

	// get_app()->clip_cursor( true );

	save_data_ = new Config();
	save_data_->load_file( "save/blue-sky.save" );

	// Direct3D
	direct_3d_ = new Direct3D11(
		get_app()->GetWindowHandle(),
		get_app()->get_width(),
		get_app()->get_height(),
		get_app()->is_full_screen(),
		0,
		0,
		get_config()->get( "video.multisample.count", 4 ), 
		get_config()->get( "video.multisample.quality", 2 )
	);
	direct_3d_->getEffect()->load( "media/shader/main.fx" );
	direct_3d_->create_default_input_layout();

	if ( get_config()->get( "video.font_enabled", 1 ) )
	{
		direct_3d_->setup_font();
	}

	game_constant_buffer_ = new GameConstantBuffer( direct_3d_.get() );
	frame_constant_buffer_ = new FrameConstantBuffer( direct_3d_.get() );
	frame_drawing_constant_buffer_ = new FrameDrawingConstantBuffer( direct_3d_.get() );
	object_constant_buffer_ = new ObjectConstantBuffer( direct_3d_.get() );

	physics_ = new ActiveObjectPhysics();
	bullet_debug_draw_ = new Direct3D11BulletDebugDraw( direct_3d_.get() );

	if ( get_config()->get< int >( "video.bullet_debug_draw", 0 ) )
	{
		bullet_debug_draw_->setDebugMode( btIDebugDraw::DBG_DrawWireframe );
		physics_->setDebugDrawer( bullet_debug_draw_.get() );
	}
	else
	{
		bullet_debug_draw_->setDebugMode( 0 );
	}

	direct_input_ = new DirectInput( get_app()->GetInstanceHandle(), get_app()->GetWindowHandle() );
	input_ = new Input();
	input_->set_direct_input( direct_input_.get() );
	input_->load_config( * get_config() );

	// Oculus Rift
	if ( get_config()->get( "input.oculus_rift.enabled", 1 ) )
	{
		oculus_rift_ = new OculusRift( direct_3d_.get() );
	}

	graphics_manager_ = new Direct3D11GraphicsManager( direct_3d_.get() );

	sound_manager_ = new SoundManager( get_app()->GetWindowHandle() );
	sound_manager_->set_mute( get_config()->get( "audio.mute", 0 ) != 0 );
	sound_manager_->set_volume( get_config()->get( "audio.volume", 1.f ) );
	
	sound_manager_->load( "ok" );
	sound_manager_->load( "cancel" );
	sound_manager_->load( "click" );

	active_object_manager_ = new ActiveObjectManager();
	drawing_model_manager_ = new DrawingModelManager();

	// MainLoop
	main_loop_ = new MainLoop( 60 );

	is_display_fps_ = get_config()->get( "video.display_fps", 0 ) != 0;
}

//■デストラクタ
GameMain::~GameMain()
{
	get_config()->set< int >( "audio.mute", sound_manager_->is_mute() );
	get_config()->set< int >( "video.full_screen", get_direct_3d()->is_full_screen() );

	get_config()->save_file( "blue-sky.config" );

	scene_.release();
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
	
	if ( get_app()->is_active() )
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

	if ( input_->push( Input::ESCAPE ) )
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

	scene_->update();

	render();

	check_scene_transition();

	return true;
}

void GameMain::render()
{
	scene_->render();

	direct_3d_->end();
}

void GameMain::on_function_key_down( int function_key )
{
	if ( function_key == 2 )
	{
		get_sound_manager()->set_mute( ! get_sound_manager()->is_mute() );
		game::Sound* bgm = get_sound_manager()->get_sound( "bgm" );

		if ( bgm )
		{
			bgm->play( true );
		}
	}

	if ( function_key == 5 )
	{
		get_direct_3d()->switch_full_screen();
		get_app()->set_full_screen( get_direct_3d()->is_full_screen() );
	}

	scene_->on_function_key_down( function_key );
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
		set_stage_name( "0-0" );
		setup_scene( "game_play" );

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
	scene_.release();

	sound_manager_->pop_group();
	sound_manager_->push_group( scene_name.c_str() );
	
	if ( true )
	{
		scene_ = new CanvasTestScene( this );
	}
	else if ( scene_name == "title" )
	{
		scene_ = new TitleScene( this );
	}
	else if ( scene_name == "stage_select" )
	{
		scene_ = new StageSelectScene( this );
	}
	else if ( scene_name == "stage_intro" )
	{
		scene_ = new StoryTextScene( this, ( std::string( "media/stage/" ) + get_stage_name() + ".intro" ).c_str(), "game_play" );
	}
	else if ( scene_name == "game_play" )
	{
		scene_ = new GamePlayScene( this );
	}
	else if ( scene_name == "stage_outro" )
	{
		scene_ = new StoryTextScene( this, ( std::string( "media/stage/" ) + get_stage_name() + ".outro" ).c_str(), "stage_select" );
	}
	else if ( scene_name == "ending" )
	{
		scene_ = new EndingScene( this );
	}
	else
	{
		COMMON_THROW_EXCEPTION_MESSAGE( std::string( "worng next_scene : " ) + scene_name );
	}

	scene_->set_name( scene_name );
	scene_->set_next_stage_name( get_stage_name() );

	get_app()->clip_cursor( scene_->is_clip_cursor_required() );
}

float_t GameMain::get_elapsed_time() const
{
	return get_main_loop()->get_elapsed_sec();
}

} // namespace blue_sky