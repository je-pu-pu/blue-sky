#include "Scene.h"
#include "GameMain.h"

namespace blue_sky
{

Scene::Scene( const GameMain* game_main )
	: game_main_( game_main )
{
}

Scene::~Scene()
{

}

/*
bool Scene::is_first_game_play() const
{
	return game_main_->is_first_game_play();
}

unsigned int Scene::get_current_time() const
{
	return game_main_->get_current_time();
}
*/

Scene::Direct3D* Scene::direct_3d() const
{
	return game_main_->get_direct_3d();
}

ActiveObjectManager* Scene::active_object_manager() const
{
	return game_main_->get_active_object_manager();
}

SoundManager* Scene::sound_manager() const
{
	return game_main_->get_sound_manager();
}

Input* Scene::input() const
{
	return game_main_->get_input();
}

Scene::Config* Scene::config() const
{
	return game_main_->get_config();
}

Scene::Config* Scene::save_data() const
{
	return game_main_->get_save_data();
}

int Scene::get_width() const
{
	return game_main_->get_width();
}

int Scene::get_height() const
{
	return game_main_->get_height();
}

/*
const std::string& Scene::get_stage_name() const
{
	return game_main_->get_stage_name();
}
*/

} // namespace blue_sky
