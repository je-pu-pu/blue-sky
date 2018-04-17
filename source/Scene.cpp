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
	update_total_elapsed_time();
}

bool Scene::is_first_game_play() const
{
	return false;

	// return game_main_->is_first_game_play();
}

void Scene::update_total_elapsed_time()
{
	total_elapsed_time_ += get_main_loop()->get_elapsed_sec();
}

float Scene::get_elapsed_time() const
{
	return get_main_loop()->get_elapsed_sec();
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

OculusRift* Scene::get_oculus_rift() const
{
	return game_main_->get_oculus_rift();
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

void Scene::update_constant_buffer_for_sprite_frame( int line_type, float_t drawing_accent_scale )
{
	{
		FrameConstantBufferData frame_constant_buffer_data;

		frame_constant_buffer_data.view = Matrix().set_identity().transpose();
		frame_constant_buffer_data.projection = Matrix().set_orthographic( 2.f * get_width() / get_height(), 2.f, 0.f, 1.f ).transpose();
		frame_constant_buffer_data.time = get_total_elapsed_time();
	
		get_game_main()->get_frame_constant_buffer()->update( & frame_constant_buffer_data );
	}

	{
		FrameDrawingConstantBufferData frame_drawing_constant_buffer_data;

		frame_drawing_constant_buffer_data.accent = get_bgm()->get_current_peak_level() * drawing_accent_scale;
		frame_drawing_constant_buffer_data.line_type = line_type;

		get_game_main()->get_frame_drawing_constant_buffer()->update( & frame_drawing_constant_buffer_data );
	}

	{
		ObjectConstantBufferData object_constant_buffer_data;

		object_constant_buffer_data.world = Matrix().set_identity().transpose();

		get_game_main()->get_object_constant_buffer()->update( & object_constant_buffer_data );
	}
}

/**
 * 指定したテクニックの全てのパスでレンダリング処理を実行する
 *
 * @param technique_name テクニック名
 * @param function レンダリング処理
 */
void Scene::render_technique( const char_t* technique_name, std::function< void() > function ) const
{
	Direct3D::EffectTechnique* technique = get_direct_3d()->getEffect()->getTechnique( technique_name );
	
	if ( ! technique )
	{
		return;
	}

	for ( auto i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
	{
		( *i )->apply();

		function();
	}
}

void Scene::render_fader() const
{
	ObjectConstantBufferData buffer_data;
	buffer_data.world = Matrix().set_identity().transpose();
	buffer_data.color = get_direct_3d()->getFader()->get_color();
	get_game_main()->get_object_constant_buffer()->update( & buffer_data );

	get_direct_3d()->setInputLayout( "main" );
	auto technique = get_direct_3d()->getEffect()->getTechnique( "|main2d" );

	for ( auto i = technique->getPassList().begin(); i !=  technique->getPassList().end(); ++i )
	{
		( *i )->apply();

		get_game_main()->get_object_constant_buffer()->bind_to_vs();
		get_game_main()->get_object_constant_buffer()->bind_to_ps();

		get_direct_3d()->getFader()->render();
	}
}

} // namespace blue_sky
