#include "ScriptBindings.h"

#include "ScriptManager.h"
#include "ObjectFactory.h"
#include "GameObjectRef.h"

#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/graphics/Model.h>
#include <blue_sky/DebugLog.h>

#include <blue_sky/Fader.h>
#include <core/graphics/DebugDraw.h>
#include <core/graphics/Shader.h>
#include <core/sound/SoundManager.h>
#include <core/sound/Sound.h>

#include <stdexcept>

namespace blue_sky
{

namespace
{

/**
 * スクリプトから渡されたオブジェクトが nil なら、落ちずにスクリプトのエラーにする（ScriptManager::exec が ScriptError として投げる）
 *
 * 以前は nil のまま使って落ちていた（set_loc( nil, … ) など）
 */
template< typename Type >
Type* require_not_nil( Type* p, const char* function_name )
{
	if ( ! p )
	{
		throw std::invalid_argument( std::string( function_name ) + " : 引数が nil" );
	}

	return p;
}

} // namespace

/**
 * ゲーム全体で共有可能なスクリプトコマンドを準備する
 *
 * 登録した関数は、受け取った窓口（targets）をコピーして持つ
 */
void ScriptBindings::bind( ScriptManager& script, const Targets& targets )
{
	auto* const graphics_manager = targets.graphics;
	auto* const sound_manager = targets.sound;
	auto* const objects = targets.objects;
	auto* const debug_draw = targets.debug_draw;
	auto* const fader = targets.fader;
	const auto request_scene = targets.request_scene;

	script.exec( "function load( s ) dofile( 'media/script/' .. s ) end" );

	// Scene
	// 遷移はその場で実行せず、要求として積む（スクリプトがシーンの更新中に呼ばれても、実行中のシーンを破棄しないように）
	// 名前の後に "?キー=値" で引数を付けられる（例 : push_overlay( "options?tab=controls" )）
	script.set_function( "scene", [request_scene] ( const char_t* name ) { SceneRequest r { SceneRequest::Type::REPLACE_BASE }; split_scene_name( name, r.name, r.args ); request_scene( r ); } );
	script.set_function( "push_overlay", [request_scene] ( const char_t* name ) { SceneRequest r { SceneRequest::Type::PUSH_OVERLAY }; split_scene_name( name, r.name, r.args ); request_scene( r ); } );
	script.set_function( "pop_overlay", [request_scene] () { request_scene( SceneRequest{ SceneRequest::Type::POP_OVERLAY } ); } );

	// Basic
	script.set_function( "color", [] ( float_t r, float_t g, float_t b, float_t a ) { return Color( r, g, b, a ); } );
	script.set_function( "vector", [] ( float_t x, float_t y, float_t z ) { return Vector( x, y, z ); } );

	// Fade
	script.set_function( "set_fade_color", [fader] ( const Color& color ) { fader->set_color( color ); } );
	script.set_function( "start_fade_in", [fader] ( float_t speed ) { fader->start_fade_in( speed ); } );
	script.set_function( "start_fade_out", [fader] ( float_t speed ) { fader->start_fade_out( speed ); } );

	// ゲームの物体（Lua の "GameObject" は EntityId を包んだ GameObjectRef。ActiveObject を持たない ECS だけの物体も同じように触れる）
	// 知らない名前なら nil を返す（以前は null に restart を呼んで落ちた）
	script.set_function( "create_object", [objects] ( const char_t* name ) -> sol::optional< GameObjectRef >
	{
		auto id = objects->create_object( name );

		if ( ! id )
		{
			return sol::nullopt;
		}

		GameObjectRef o = objects->get_ref( * id );
		o.restart();

		return o;
	} );
	script.set_function( "clone_object", [] ( const GameObjectRef* o ) -> sol::optional< GameObjectRef >
	{
		// 以前は ActiveObject の種類だけ複製できた（段階 6 で最後の種類のプレイヤーも ECS だけの物体になり、複製できる物体は無くなった）。
		// ECS だけの物体は create_object で作り直す。黙って無視しない
		require_not_nil( o, "clone_object" );
		debug_log( "[Object] clone_object : 物体は複製できません（create_object で作り直してください）" );

		return sol::nullopt;
	} );
	script.set_function( "get_object", [objects] ( const char_t* name ) -> sol::optional< GameObjectRef >
	{
		if ( auto o = objects->find_ref( name ) )
		{
			return * o;
		}

		return sol::nullopt;
	} );
	script.set_function( "set_name", [objects] ( const GameObjectRef* o, const char_t* name ) { objects->name( name, require_not_nil( o, "set_name" )->get_entity_id() ); } );
	script.set_function( "set_loc", [] ( GameObjectRef* o, float_t x, float_t y, float_t z ) { require_not_nil( o, "set_loc" )->set_location( x, y, z ); } );
	script.set_function( "set_dir", [] ( GameObjectRef* o, float_t r ) { require_not_nil( o, "set_dir" )->set_direction_degree( r ); } );
	script.set_function( "set_vel", [] ( GameObjectRef* o, float_t x, float_t y, float_t z ) { require_not_nil( o, "set_vel" )->set_velocity( Vector( x, y, z ) ); } );
	script.set_function( "set_start_loc", [] ( GameObjectRef* o, float_t  x, float_t  y, float_t  z ) { require_not_nil( o, "set_start_loc" )->set_start_location( x, y, z ); } );
	script.set_function( "set_start_rot", [] ( GameObjectRef* o, float_t rx, float_t ry, float_t rz ) { require_not_nil( o, "set_start_rot" )->set_start_rotation( rx, ry, rz ); } );
	script.set_function( "set_start_dir", [] ( GameObjectRef* o, float_t r ) { require_not_nil( o, "set_start_dir" )->set_start_direction_degree( r ); } );
	script.set_function( "set_mass", [] ( GameObjectRef* o, float_t mass ) { require_not_nil( o, "set_mass" )->set_mass( mass ); } );
	script.set_function( "make_model_unique", [] ( GameObjectRef* o ) { return require_not_nil( o, "make_model_unique" )->make_model_unique(); } );

	// GrahpicsManager
	script.set_function( "setup_shadow_map", [graphics_manager] ( uint_t level = 3, uint_t size = 1024 ) { graphics_manager->setup_shadow_map( level, size ); } );
	script.set_function( "set_sky_box", [graphics_manager] ( const char_t* name ) { graphics_manager->set_sky_box( name ); } );
	script.set_function( "unset_sky_box", [graphics_manager] () { graphics_manager->unset_sky_box(); } );
	script.set_function( "set_ground", [graphics_manager] ( const char_t* name ) { graphics_manager->set_ground( name ); } );
	script.set_function( "unset_ground", [graphics_manager] () { graphics_manager->unset_ground(); } );

	script.set_function( "set_paper_texture_type", [graphics_manager] ( int_t type ) { graphics_manager->set_paper_texture_type( type ); } );
	script.set_function( "set_ambient_color", [graphics_manager] ( const Color& color ) { graphics_manager->set_ambient_color( color ); } );
	script.set_function( "set_shadow_color", [graphics_manager] ( const Color& color ) { graphics_manager->set_shadow_color( color ); } );
	script.set_function( "set_shadow_paper_color", [graphics_manager] ( const Color& color ) { graphics_manager->set_shadow_paper_color( color ); } );
	script.set_function( "set_drawing_accent", [graphics_manager] ( float_t accent ) { graphics_manager->set_drawing_accent( accent ); } );
	script.set_function( "set_drawing_line_type", [graphics_manager] ( int_t type ) { graphics_manager->set_drawing_line_type( type ); } );
	script.set_function( "set_light_direction", [graphics_manager] ( float_t x, float_t y, float_t z ) { return graphics_manager->set_light_direction( Vector( x, y, z, 0.f ) ); } );

	script.set_function( "set_post_effect_shader", [graphics_manager] ( core::graphics::Shader* s ) { graphics_manager->set_post_effect_shader( require_not_nil( s, "set_post_effect_shader" ) ); } );

	script.set_function( "load_model", [graphics_manager] ( const char_t* name ) { return graphics_manager->load_model( name ); } );
	script.set_function( "clone_model", [graphics_manager] ( graphics::Model* m ) { auto* m2 = graphics_manager->clone_model( require_not_nil( m, "clone_model" ) ); return m2; } );
	script.set_function( "clone_model_with_shaders", [graphics_manager] ( graphics::Model* m ) { return graphics_manager->clone_model_with_shaders( require_not_nil( m, "clone_model_with_shaders" ) ); } );

	script.set_function( "get_shader", [graphics_manager] ( const char_t* name ) { return graphics_manager->get_shader( name ); } );
	script.set_function( "clone_shader", [graphics_manager] ( const core::graphics::Shader* s ) { return graphics_manager->clone_shader( require_not_nil( s, "clone_shader" ) ); } );

	script.set_function( "load_texture", [graphics_manager] ( const char_t* file_path ) { return graphics_manager->load_texture( file_path ); } );
	// script.set_function( "load_named_texture", [graphics_manager] ( const char_t* name, const char_t* file_path ) { return graphics_manager->load_texture( name, file_path ); } );
	script.set_function( "get_texture", [graphics_manager] ( const char_t* name ) { return graphics_manager->get_texture( name ); } );

	// SoundManager
	script.set_function( "play_sound", [sound_manager] ( const char_t* name ) { if ( auto* s = sound_manager->get_sound( name ) ) { s->play( false ); } } );

	// debug
	script.set_function( "debug_axis", [graphics_manager] ( int on ) { graphics_manager->set_debug_axis_enabled( on != 0 ); } );
	script.set_function( "debug_bullet", [debug_draw] ( int mode ) { debug_draw->set_physics_debug_mode( mode ); } );

	script.set_function( "debug_print_resources", [graphics_manager] () { graphics_manager->debug_print_resources(); } );
}

} // namespace blue_sky
