#include "Scene.h"

#include <GameMain.h>
#include <ActiveObjectManager.h>
#include <ActiveObjectPhysics.h>

#include <blue_sky/graphics/GraphicsManager.h>

#include <core/sound/SoundManager.h>
#include <core/sound/Sound.h>

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

ActiveObjectManager* Scene::get_active_object_manager() const
{
	return game_main_->get_active_object_manager();
}

Scene::PhysicsManager* Scene::get_physics_manager() const
{
	return game_main_->get_physics_manager();
}

GraphicsManager* Scene::get_graphics_manager() const
{
	return game_main_->get_graphics_manager();
}

Scene::SoundManager* Scene::get_sound_manager() const
{
	return game_main_->get_sound_manager();
}

ScriptManager* Scene::get_script_manager() const
{
	return game_main_->get_script_manager();
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

/***
 * スプライト描画のための定数バッファを更新する
 *
 * @todo GraphicsManager に移行する
 */
void Scene::update_constant_buffer_for_sprite_frame( int line_type, float_t drawing_accent_scale )
{
	{
		auto& frame_constant_buffer_data = get_graphics_manager()->get_frame_render_data()->data();

		frame_constant_buffer_data.view = Matrix().set_identity().transpose();
		frame_constant_buffer_data.projection = Matrix().set_orthographic( 2.f * get_width() / get_height(), 2.f, 0.f, 1.f ).transpose();
		frame_constant_buffer_data.time = get_total_elapsed_time();
	
		get_graphics_manager()->get_frame_render_data()->update();
	}

	{
		get_graphics_manager()->set_drawing_accent( get_bgm()->get_current_peak_level() * drawing_accent_scale );
		get_graphics_manager()->set_drawing_line_type( line_type );

		get_graphics_manager()->get_frame_drawing_render_data()->update();
	}

	{
		ObjectConstantBufferData object_constant_buffer_data;

		object_constant_buffer_data.world = Matrix().set_identity().transpose();

		get_graphics_manager()->get_shared_object_render_data()->update( & object_constant_buffer_data );
	}
}

/**
 * 全ての定数バッファをバインドする
 *
 */
void Scene::bind_all_render_data() const
{
	get_graphics_manager()->get_game_render_data()->bind_to_all();
	get_graphics_manager()->get_frame_render_data()->bind_to_all();
	get_graphics_manager()->get_frame_drawing_render_data()->bind_to_all();
	get_graphics_manager()->get_shared_object_render_data()->bind_to_all();
}

/**
 * ゲーム毎の定数バッファをバインドする
 *
 */
void Scene::bind_game_render_data() const
{
	get_graphics_manager()->get_game_render_data()->bind_to_vs();
	get_graphics_manager()->get_game_render_data()->bind_to_gs();
	get_graphics_manager()->get_game_render_data()->bind_to_ps();
}

/**
 * フレーム毎の定数バッファをバインドする
 *
 */
void Scene::bind_frame_render_data() const
{
	get_graphics_manager()->get_frame_render_data()->bind_to_vs();
	get_graphics_manager()->get_frame_render_data()->bind_to_gs();
	get_graphics_manager()->get_frame_render_data()->bind_to_ps();
}

/**
 * オブジェクト毎の定数バッファをバインドする
 *
 */
void Scene::bind_shared_object_render_data() const
{
	get_graphics_manager()->get_shared_object_render_data()->bind_to_vs();
	get_graphics_manager()->get_shared_object_render_data()->bind_to_ds();
	get_graphics_manager()->get_shared_object_render_data()->bind_to_ps();
}

/**
 * 指定したテクニックの全てのパスでレンダリング処理を実行する
 *
 * @param technique_name テクニック名
 * @param function レンダリング処理
 */
void Scene::render_technique( const char_t* technique_name, std::function< void() > function ) const
{
	get_graphics_manager()->render_technique( technique_name, function  );
}

/**
 * フェーダーを描画する
 *
 */
void Scene::render_fader() const
{
	get_graphics_manager()->render_fader();
}

} // namespace blue_sky
