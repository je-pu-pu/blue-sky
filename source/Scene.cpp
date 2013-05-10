#include "Scene.h"
#include "GameMain.h"

#include "ConstantBuffer.h"
#include "Direct3D11ConstantBuffer.h"
#include "Direct3D11Effect.h"
#include "Direct3D11Fader.h"
#include "Direct3D11.h"

#include "SoundManager.h"
#include "Sound.h"

#include <game/MainLoop.h>

namespace blue_sky
{

Scene::Scene( const GameMain* game_main )
	: game_main_( game_main )
	, total_elapsed_time_( 0.f )
{
}

Scene::~Scene()
{

}

void Scene::update()
{
	total_elapsed_time_ += get_main_loop()->get_elapsed_sec();
}

bool Scene::is_first_game_play() const
{
	return false;

	// return game_main_->is_first_game_play();
}

float Scene::get_elapsed_time() const
{
	return game_main_->get_elapsed_time();
}

Scene::Direct3D* Scene::get_direct_3d() const
{
	return game_main_->get_direct_3d();
}

Scene::Physics* Scene::get_physics() const
{
	return game_main_->get_physics();
}

ActiveObjectManager* Scene::get_active_object_manager() const
{
	return game_main_->get_active_object_manager();
}

DrawingModelManager* Scene::get_drawing_model_manager() const
{
	return game_main_->get_drawing_model_manager();
}

GraphicsManager* Scene::get_graphics_manager() const
{
	return game_main_->get_graphics_manager();
}

SoundManager* Scene::get_sound_manager() const
{
	return game_main_->get_sound_manager();
}

Input* Scene::get_input() const
{
	return game_main_->get_input();
}

Scene::Config* Scene::get_config() const
{
	return game_main_->get_config();
}

Scene::Config* Scene::get_save_data() const
{
	return game_main_->get_save_data();
}

const Scene::MainLoop* Scene::get_main_loop() const
{
	return game_main_->get_main_loop();
}

const App* Scene::get_app() const
{
	return game_main_->get_app();
}

int Scene::get_width() const
{
	return game_main_->get_width();
}

int Scene::get_height() const
{
	return game_main_->get_height();
}

const std::string& Scene::get_stage_name() const
{
	return game_main_->get_stage_name();
}

void Scene::play_sound( const char* name, bool loop, bool force ) const
{
	Sound* sound = GameMain::get_instance()->get_sound_manager()->get_sound( name );
	
	if ( sound )
	{
		if ( force || ! sound->is_playing() )
		{
			sound->play( loop );
		}
	}
}

void Scene::stop_sound( const char* name ) const
{
	Sound* sound = GameMain::get_instance()->get_sound_manager()->get_sound( name );
	
	if ( sound )
	{
		sound->stop();
	}
}

void Scene::render_fader() const
{
	ObjectConstantBufferData buffer_data;

	buffer_data.color = get_direct_3d()->getFader()->get_color();

	get_game_main()->get_object_constant_buffer()->update( & buffer_data );

	get_direct_3d()->setInputLayout( "main" );
	Direct3D::EffectTechnique* technique = get_direct_3d()->getEffect()->getTechnique( "|main2d" );

	for ( Direct3D::EffectTechnique::PassList::iterator i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
	{
		( *i )->apply();
				
		get_game_main()->get_object_constant_buffer()->bind_to_ps();

		get_direct_3d()->getFader()->render();
	}
}

} // namespace blue_sky
