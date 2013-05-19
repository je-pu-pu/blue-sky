#include "GameMain.h"
#include "App.h"

#include "TitleScene.h"
#include "StageSelectScene.h"
#include "StoryTextScene.h"
#include "GamePlayScene.h"

#include "DrawingModelManager.h"
#include "ActiveObjectManager.h"

#include "ConstantBuffer.h"

#include "Direct3D11.h"
#include "Direct3D11GraphicsManager.h"

#include "Direct3D11MeshManager.h"
#include "Direct3D11ConstantBuffer.h"
#include "Direct3D11BulletDebugDraw.h"
#include "Direct3D11Effect.h"

#include "DirectInput.h"
#include "Input.h"

#include "ActiveObjectPhysics.h"

#include "SoundManager.h"
#include "Sound.h"

#include "DirectX.h"

#include <win/Version.h>

#include <game/Config.h>
#include <game/MainLoop.h>

#include <common/math.h>
#include <common/log.h>

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
	win::Version version;
	version.log( "log/windows_version.log" );

	// get_app()->clip_cursor( true );

	// Config
	config_ = new Config();
	config_->load_file( "blue-sky.config" );

	save_data_ = new Config();
	save_data_->load_file( "save/blue-sky.save" );

	// App
	get_app()->set_size( get_config()->get( "video.width", App::DEFAULT_WIDTH ), get_config()->get( "video.height", App::DEFAULT_HEIGHT ) );
	get_app()->set_full_screen( get_config()->get( "video.full_screen", 0 ) != 0 );

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
	input_->load_config( * config_.get() );

	graphics_manager_ = new Direct3D11GraphicsManager( direct_3d_.get() );

	sound_manager_ = new SoundManager( get_app()->GetWindowHandle() );
	sound_manager_->set_mute( config_->get( "audio.mute", 0 ) != 0 );
	sound_manager_->set_volume( config_->get( "audio.volume", 1.f ) );

	active_object_manager_ = new ActiveObjectManager();
	drawing_model_manager_ = new DrawingModelManager();

	// Scene
	scene_ = new TitleScene( this );
	scene_->set_name( "title" );

	// scene_ = new StoryTextScene( this, "media/stage/8.outro", "title" );
	// scene_->set_name( "stage_outro" );

	// MainLoop
	main_loop_ = new MainLoop( 60 );

	is_display_fps_ = config_->get( "video.display_fps", 0 ) != 0;
}

//■デストラクタ
GameMain::~GameMain()
{
	config_->set< int >( "audio.mute", sound_manager_->is_mute() );
	config_->set< int >( "video.full_screen", get_direct_3d()->is_full_screen() );

	config_->save_file( "blue-sky.config" );

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
			scene_->set_next_scene( "title" );
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
	std::string next_scene = scene_->get_next_scene();

	if ( ! next_scene.empty() )
	{
		set_stage_name( scene_->get_next_stage_name() );

		scene_.release();

		if ( next_scene == "title" )
		{
			scene_ = new TitleScene( this );
		}
		else if ( next_scene == "stage_select" )
		{
			scene_ = new StageSelectScene( this );
		}
		else if ( next_scene == "stage_intro" )
		{
			scene_ = new StoryTextScene( this, ( std::string( "media/stage/" ) + get_stage_name() + ".intro" ).c_str(), "game_play" );
		}
		else if ( next_scene == "game_play" )
		{
			scene_ = new GamePlayScene( this );
		}
		else if ( next_scene == "stage_outro" )
		{
			scene_ = new StoryTextScene( this, ( std::string( "media/stage/" ) + get_stage_name() + ".outro" ).c_str(), "stage_select" );
		}
		else
		{
			COMMON_THROW_EXCEPTION_MESSAGE( std::string( "worng next_scene : " ) + next_scene );
		}

		scene_->set_name( next_scene );
		scene_->set_next_stage_name( get_stage_name() );

		get_app()->clip_cursor( scene_->is_clip_cursor_required() );

		save_data_->save_file( "save/blue-sky.save" );
	}
}

float_t GameMain::get_elapsed_time() const
{
	return get_main_loop()->get_elapsed_sec();
}

} // namespace blue_sky