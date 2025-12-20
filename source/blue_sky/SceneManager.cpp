#include "SceneManager.h"

#include <Scene/TitleScene.h>
#include <Scene/StageSelectScene.h>
#include <Scene/GamePlayScene.h>
#include <Scene/StoryTextScene.h>
#include <Scene/EndingScene.h>

#include <Scene/DebugScene.h>
#include <Scene/CanvasTestScene.h>
#include <Scene/TransformTestScene.h>
#include <Scene/ParticleSystemTestScene.h>
#include <Scene/GeometryShaderCanvasTestScene.h>

#include <Scene/PhysicsTestScene.h>

#include <blue_sky/GameMain.h>

namespace blue_sky
{

/**
 * 全てのシーンを登録する
 * 
 * 新しいシーンを追加した場合は、ここに登録コードを追加する必要がある
 * 
 * @todo シーンの自動登録機能を実装する
 */
void SceneManager::register_all_scene()
{
	register_scene< TitleScene >();
	register_scene< StageSelectScene >();
	register_scene< GamePlayScene >();

	register_scene< StoryTextScene >( "stage_intro", [] () { return new StoryTextScene( ( std::string( "media/stage/" ) + GameMain::get_instance()->get_stage_name() + ".intro" ).c_str(), "game_play" ); } );
	register_scene< StoryTextScene >( "stage_outro", [] () { return new StoryTextScene( ( std::string( "media/stage/" ) + GameMain::get_instance()->get_stage_name() + ".outro" ).c_str(), "stage_select" ); } );

	register_scene< EndingScene >();

	register_scene< DebugScene >();
	register_scene< CanvasTestScene >();
	register_scene< TransformTestScene >();
	register_scene< ParticleSystemTestScene >();
	register_scene< GeometryShaderCanvasTestScene >();

	register_scene< PhysicsTestScene >();
}

} // namespace blue_sky
