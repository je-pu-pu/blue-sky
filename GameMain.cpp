/**
 *
 *
 */


#include "GameMain.h"

#include "App.h"

#include "TitleScene.h"
#include "StageSelectScene.h"
#include "GamePlayScene.h"

#include "Input.h"
#include "SoundManager.h"
#include "GridDataManager.h"
#include "GridObjectManager.h"
#include "ActiveObjectManager.h"

#include "Direct3D9.h"
#include "Direct3D9Font.h"
#include "DirectInput.h"
#include "DirectX.h"

#include <game/Sound.h>
#include <game/Config.h>

#include <common/exception.h>
#include <common/serialize.h>
#include <common/random.h>
#include <common/math.h>

#include <boost/format.hpp>

namespace blue_sky
{

//■コンストラクタ
GameMain::GameMain()
	: app_( App::GetInstance() )
	, direct_3d_( 0 )
	, direct_input_( 0 )
	, input_( 0 )
	, sound_manager_( 0 )
	, grid_data_manager_( 0 )
	, grid_object_manager_( 0 )
	, active_object_manager_( 0 )
	, config_( 0 )
	, scene_( 0 )
{
	// ランダマイズ
	srand( timeGetTime() );

	// Config
	config_ = new Config();
	config_->load_file( "blue-sky.config" );

	// App
	app_->set_size( config_->get( "video.width", App::DEFAULT_WIDTH ), config_->get( "video.height", App::DEFAULT_HEIGHT ) );
	app_->set_full_screen( config_->get( "video.full_screen", 0 ) != 0 );

	// Direct3D
	direct_3d_ = new Direct3D9( app_->GetWindowHandle(), app_->get_width(), app_->get_height(), app_->is_full_screen(), config_->get( "video.multi_sample_type", 0 ), config_->get( "video.multi_sample_quality", 0 ) );
	direct_3d_->load_effect_file( "media/shader/blue-sky.fx" );

	if ( app_->is_full_screen() )
	{
		Sleep( 1000 );
	}

	// DirectInput
	direct_input_ = new DirectInput( app_->GetInstanceHandle(), app_->GetWindowHandle() );

	// Input
	input_ = new Input();
	input_->set_direct_input( direct_input_ );
	input_->load_config( * config_ );

	// Sound
	sound_manager_ = new SoundManager( app_->GetWindowHandle() );
	sound_manager_->set_enabled( config_->get( "audio.enable", 1 ) != 0 );

	// GridDataManager
	grid_data_manager_ = new GridDataManager();

	// GridOBjectManager
	grid_object_manager_ = new GridObjectManager();

	// ActiveObjectManager 
	active_object_manager_ = new ActiveObjectManager();

	// Scene
	scene_ = new TitleScene( this );

	/*
	// DirectShow
	IGraphBuilder

	CoInitialize( 0 );
	CoCreateInstance( CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, static_cast< void** >( m_cpGraph.ToCreator() ) );
	*/
}

//■デストラクタ
GameMain::~GameMain()
{
	config_->set< int >( "audio.enable", sound_manager_->is_enabled() );
	config_->set< int >( "video.full_screen", app_->is_full_screen() );
	config_->save_file( "blue-sky.config" );

	delete scene_;

	delete direct_3d_;

	delete input_;

	delete direct_input_;

	delete sound_manager_;

	delete grid_data_manager_;

	delete grid_object_manager_;

	delete active_object_manager_;

	delete config_;
}

/**
 * メインループ処理
 *
 */
bool GameMain::update()
{
	MainLoop.WaitTime = 15;

	// FPS 固定
	if ( ! MainLoop.Loop() )
	{
		return false ;
	}
	
	direct_input_->update();
	input_->update();
	sound_manager_->update();

	scene_->update();

	std::string next_scene = scene_->get_next_scene();

	if ( ! next_scene.empty() )
	{
		delete scene_;
		scene_ = 0;

		if ( next_scene == "title" )
		{
			scene_ = new TitleScene( this );
		}
		else if ( next_scene == "stage_select" )
		{
			scene_ = new StageSelectScene( this );
		}
		else if ( next_scene == "game_play" )
		{
			scene_ = new GamePlayScene( this );
		}
		else
		{
			COMMON_THROW_EXCEPTION_MESSAGE( std::string( "worng next_scene : " + scene_->get_next_scene() ) );
		}

		app_->clip_cursor( scene_->is_clip_cursor_required() );
	}

	render();

	return true;
}

/**
 * 描画
 */
void GameMain::render()
{
	const int render_div = 1;
	static int n = 0;

	if ( n < render_div - 1 )
	{
		n++;
		return;
	}

	n = 0;

	scene_->render();

	// Debug
	std::string debug_text;
	debug_text = std::string( "FPS : " ) + common::serialize( MainLoop.GetFPS() );
	get_direct_3d()->getFont()->draw_text( 0, 0, debug_text.c_str(), D3DCOLOR_XRGB( 0, 0, 0 ) );

	HRESULT hr = get_direct_3d()->getDevice()->Present( NULL, NULL, NULL, NULL );

	if ( hr == D3DERR_DEVICELOST )
	{
		get_direct_3d()->reset();
	}
	else
	{
		DIRECT_X_FAIL_CHECK( hr );
	}
}

bool GameMain::is_first_game_play() const
{
	return false;
}



int GameMain::get_width() const
{
	return app_->get_width();
}

int GameMain::get_height() const
{
	return app_->get_height();
}

void GameMain::on_reset_key_down()
{
	app_->close();

	/*
	if ( scene_->get_name() == "title" )
	{
		
	}
	*/
}

void GameMain::on_function_key_down( int function_key )
{
	if ( function_key == 1 ) 
	{
		get_sound_manager()->stop_all();
		scene_->set_next_scene( "title" );
	}

	if ( function_key == 2 )
	{
		get_sound_manager()->set_enabled( ! get_sound_manager()->is_enabled() );
		Sound* bgm = get_sound_manager()->get_sound( "bgm" );

		if ( bgm )
		{
			bgm->play( true );
		}
	}

	if ( function_key == 5 )
	{
		app_->set_full_screen( ! app_->is_full_screen() );
		get_direct_3d()->set_full_screen( App::GetInstance()->is_full_screen() );
	}
}

void GameMain::on_mouse_wheel( int wheel )
{
	input_->push_mouse_wheel_queue( wheel > 0 ? 1 : -1 );
}

bool GameMain::is_clip_cursor_required() const
{
	return scene_->is_clip_cursor_required();
}

} // namespace blue_sky
