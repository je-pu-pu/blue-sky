#include "Scene.h"

#include <blue_sky/GameMain.h>
#include <blue_sky/ObjectUpdater.h>
#include <blue_sky/ObjectFactory.h>
#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/DebugLog.h>
#include <blue_sky/Fader.h>

#include <core/ecs/EntityManager.h>
#include <core/sound/SoundManager.h>
#include <core/sound/Sound.h>

#include <game/MainLoop.h>

namespace blue_sky
{

Scene::Scene( GameMain& game_main )
	: game_main_( & game_main )
	, total_elapsed_time_( 0.f )
	, is_paused_( false )
	, arguments_( game_main.get_next_scene_arguments() )
{

}

Scene::Scene( const SceneArguments& arguments )
	: game_main_( nullptr )
	, total_elapsed_time_( 0.f )
	, is_paused_( false )
	, arguments_( arguments )
{

}

Scene::~Scene()
{

}

void Scene::log_lost_resource( const char_t* what ) const
{
	if ( lost_resource_logged_.insert( what ).second )
	{
		debug_log( "[" + get_name() + "] " + what + " を引けません（解放された）。使わずに続けます" );
	}
}

Scene::Texture* Scene::resolve( TextureHandle h, const char_t* what ) const
{
	Texture* t = h.get();

	if ( ! t && ! h.is_null() )
	{
		log_lost_resource( what );
	}

	return t;
}

Scene::Model* Scene::resolve( ModelHandle h, const char_t* what ) const
{
	Model* m = static_cast< Model* >( h.get() );	// GraphicsManager は blue_sky のモデルだけを入れる

	if ( ! m && ! h.is_null() )
	{
		log_lost_resource( what );
	}

	return m;
}

Scene::Sound* Scene::resolve( SoundHandle h, const char_t* what ) const
{
	Sound* s = h.get();

	if ( ! s && ! h.is_null() )
	{
		log_lost_resource( what );
	}

	return s;
}

Scene::TextureHandle Scene::load_named_texture_handle( const char_t* name, const char_t* file_name ) const
{
	return get_graphics_manager()->get_texture_handle_of( get_graphics_manager()->load_named_texture( name, file_name ) );
}

Scene::ModelHandle Scene::load_model_handle( const char_t* name ) const
{
	return get_graphics_manager()->get_model_handle( get_graphics_manager()->load_model( name ) );
}

void Scene::play( SoundHandle h, const char_t* what, bool loop ) const
{
	if ( Sound* s = resolve( h, what ) )
	{
		s->play( loop );
	}
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

Scene::EntityManager* Scene::get_entity_manager() const
{
	return game_main_->get_entity_manager();
}

ObjectUpdater* Scene::get_object_updater() const
{
	return get_game_main()->get_object_updater();
}

ObjectFactory* Scene::get_object_factory() const
{
	return get_game_main()->get_object_factory();
}

core::physics::PhysicsManager* Scene::get_physics_manager() const
{
	return game_main_->get_physics_manager();
}

const core::TimeManager* Scene::get_time_manager() const
{
	return game_main_->get_time_manager();
}

GraphicsManager* Scene::get_graphics_manager() const
{
	return get_game_main()->get_graphics_manager();
}

Scene::SoundManager* Scene::get_sound_manager() const
{
	return get_game_main()->get_sound_manager();
}

ScriptManager* Scene::get_script_manager() const
{
	return get_game_main()->get_script_manager();
}

Input* Scene::get_input() const
{
	return get_game_main()->get_input();
}

Scene::Config* Scene::get_config() const
{
	return get_game_main()->get_config();
}

Scene::Config* Scene::get_save_data() const
{
	return get_game_main()->get_save_data();
}

OculusRift* Scene::get_oculus_rift() const
{
	return get_game_main()->get_oculus_rift();
}

const Scene::MainLoop* Scene::get_main_loop() const
{
	return get_game_main()->get_main_loop();
}

const App* Scene::get_app() const
{
	return get_game_main()->get_app();
}

const GameMain* Scene::get_game_main() const
{
	return game_main_;
}

core::setting::SettingStore* Scene::get_setting_store() const
{
	return game_main_->get_settings();
}

core::graphics::DebugDraw* Scene::get_debug_draw() const
{
	return game_main_->get_debug_draw();
}

Fader* Scene::get_fader() const
{
	return game_main_->get_fader();
}

int Scene::get_width() const
{
	return get_game_main()->get_width();
}

int Scene::get_height() const
{
	return get_game_main()->get_height();
}

const std::string& Scene::get_stage_name() const
{
	return get_game_main()->get_stage_name();
}

void Scene::play_sound( const char* name, bool loop, bool force ) const
{
	Sound* sound = get_sound_manager()->get_sound( name );
	
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
	Sound* sound = get_sound_manager()->get_sound( name );
	
	if ( sound )
	{
		sound->stop();
	}
}

/***
 * スプライト描画のための定数バッファを更新する
 */
void Scene::update_constant_buffer_for_sprite_frame( int line_type, float_t drawing_accent_scale )
{
	{
		auto& frame_constant_buffer_data = get_graphics_manager()->get_frame_render_data()->data();

		frame_constant_buffer_data.view = Matrix().set_identity();
		frame_constant_buffer_data.projection = Matrix().set_orthographic( 2.f * static_cast< float >( get_width() ) / static_cast< float >( get_height() ), 2.f, 0.f, 1.f );
		frame_constant_buffer_data.time = get_total_elapsed_time();
	
		get_graphics_manager()->get_frame_render_data()->update();
	}

	{
		const Sound* bgm = get_bgm();	// BGM が無い・引けないなら、線の強調は 0
		get_graphics_manager()->set_drawing_accent( ( bgm ? bgm->get_current_peak_level() : 0.f ) * drawing_accent_scale );
		get_graphics_manager()->set_drawing_line_type( line_type );

		get_graphics_manager()->get_frame_drawing_render_data()->update();
	}

	{
		ObjectConstantBufferData object_constant_buffer_data;

		object_constant_buffer_data.world = Matrix().set_identity();

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
	get_graphics_manager()->render_screen_color( get_fader()->get_color() );
}

} // namespace blue_sky
