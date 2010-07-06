/**
 *
 *
 */


#include "GameMain.h"

#include "App.h"

#include "TitleScene.h"
#include "GamePlayScene.h"

#include "Input.h"
#include "SoundManager.h"
#include "GridDataManager.h"
#include "GridObjectManager.h"

#include "Direct3D9.h"

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
	, input_( 0 )
	, sound_manager_( 0 )
	, grid_data_manager_( 0 )
	, grid_object_manager_( 0 )
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


	// Input
	input_ = new Input();
	input_->load_config( * config_ );

	// Sound
	sound_manager_ = new SoundManager( app_->GetWindowHandle() );

	// GridDataManager
	grid_data_manager_ = new GridDataManager();

	// GridOBjectManager
	grid_object_manager_ = new GridObjectManager();

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
	delete scene_;

	delete direct_3d_;

	delete input_;

	delete sound_manager_;

	config_->save_file( "blue-sky.config" );

	delete config_;
}

/**
 * メインループ処理
 *
 */
void GameMain::update()
{
	MainLoop.WaitTime = 18;
	MainLoop.WaitTime = 0;

	//秒間50フレームを保持
	if ( ! MainLoop.Loop() )
	{
		return;
	}
	
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
		else if ( next_scene == "game_play" )
		{
			scene_ = new GamePlayScene( this );
		}
		else
		{
			COMMON_THROW_EXCEPTION_MESSAGE( std::string( "worng next_scene : " + scene_->get_next_scene() ) );
		}
	}

	render();
}

/**
 * 描画
 */
void GameMain::render()
{
	scene_->render();

	// Debug
	
	std::string debug_text;
	debug_text = std::string( "blue-sky | FPS : " ) + common::serialize( MainLoop.GetFPS() );
	/*
	debug_text += ", player : (" + 
		common::serialize( static_cast< int >( player_->position().x() ) ) + "," +
		common::serialize( static_cast< int >( player_->position().y() ) ) + "," +
		common::serialize( static_cast< int >( player_->position().z() ) ) + ")";
	*/

	App::GetInstance()->setTitle( debug_text.c_str() );
}

int GameMain::get_width() const
{
	return app_->get_width();
}

int GameMain::get_height() const
{
	return app_->get_height();
}

} // namespace blue_sky