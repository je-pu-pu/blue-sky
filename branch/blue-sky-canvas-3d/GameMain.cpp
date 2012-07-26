#include "GameMain.h"
#include "App.h"

#include "TitleScene.h"
#include "GamePlayScene.h"

#include "DrawingModelManager.h"
#include "ActiveObjectManager.h"

#include "ConstantBuffer.h"

#include "Direct3D11.h"
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
{
	win::Version version;
	version.log( "log/windows_version.log" );

	get_app()->clip_cursor( true );

	// Config
	config_ = new Config();
	config_->load_file( "blue-sky.config" );

	save_data_ = new Config();
	save_data_->load_file( "save/blue-sky.save" );

	// MainLoop
	main_loop_ = new MainLoop( 60 );

	// Direct3D
	direct_3d_ = new Direct3D11( get_app()->GetWindowHandle(), get_app()->get_width(), get_app()->get_height(), false );
	direct_3d_->getEffect()->load( "media/shader/main.fx" );
	direct_3d_->create_default_input_layout();

	game_constant_buffer_ = new Direct3D11ConstantBuffer( direct_3d_.get(), sizeof( GameConstantBuffer ), 0 );
	frame_constant_buffer_ = new Direct3D11ConstantBuffer( direct_3d_.get(), sizeof( FrameConstantBuffer ), 1 );
	object_constant_buffer_ = new Direct3D11ConstantBuffer( direct_3d_.get(), sizeof( ObjectConstantBuffer ), 2 );

	physics_ = new ActiveObjectPhysics();
	bullet_debug_draw_ = new Direct3D11BulletDebugDraw( direct_3d_.get() );
	bullet_debug_draw_->setDebugMode( btIDebugDraw::DBG_DrawWireframe );
	bullet_debug_draw_->setDebugMode( 0 );

	physics_->setDebugDrawer( bullet_debug_draw_.get() );

	direct_input_ = new DirectInput( get_app()->GetInstanceHandle(), get_app()->GetWindowHandle() );
	input_ = new Input();
	input_->set_direct_input( direct_input_.get() );
	input_->load_config( * config_.get() );

	sound_manager_ = new SoundManager( get_app()->GetWindowHandle() );
	
	get_sound_manager()->load_music( "bgm", "tower" );
	get_sound_manager()->get_sound( "bgm" )->play( true );

	active_object_manager_ = new ActiveObjectManager();
	drawing_model_manager_ = new DrawingModelManager();

	// Scene
	scene_ = new TitleScene( this );
	scene_->set_name( "title" );
}

//■デストラクタ
GameMain::~GameMain()
{

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
		/*
		else if ( next_scene == "stage_select" )
		{
			scene_ = new StageSelectScene( this );
		}
		else if ( next_scene == "stage_intro" )
		{
			scene_ = new StoryTextScene( this, ( std::string( "media/stage/" ) + get_stage_name() + ".intro" ).c_str(), "game_play" );
		}
		*/
		else if ( next_scene == "game_play" )
		{
			scene_ = new GamePlayScene( this );
		}
		/*
		else if ( next_scene == "stage_outro" )
		{
			scene_ = new StoryTextScene( this, ( std::string( "media/stage/" ) + get_stage_name() + ".outro" ).c_str(), "stage_select" );
		}
		else if ( next_scene == "ending" )
		{
			scene_ = new EndingScene( this );
		}
		*/
		else
		{
			COMMON_THROW_EXCEPTION_MESSAGE( std::string( "worng next_scene : " + scene_->get_next_scene() ) );
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