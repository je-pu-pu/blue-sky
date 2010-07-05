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

Direct3D9* Scene::direct_3d() const
{
	return game_main_->get_direct_3d();
}

GridObjectManager* Scene::grid_object_manager() const
{
	return game_main_->get_grid_object_manager();
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

int Scene::get_width() const
{
	return game_main_->get_width();
}

int Scene::get_height() const
{
	return game_main_->get_height();
}

} // namespace blue_sky
