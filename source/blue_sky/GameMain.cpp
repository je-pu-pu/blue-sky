#include "GameMain.h"
#include "App.h"

#include "SceneManager.h"
#include "SceneStack.h"
#include "Scene/Scene.h"

#include "ObjectUpdater.h"
#include "ObjectFactory.h"

// #include "ConstantBuffer.h"

#include "Input.h"


#include "ScriptManager.h"

#include <blue_sky/graphics/Direct3D11/GraphicsManager.h>

#include <core/ecs/EntityManager.h>
#include <core/graphics/Direct3D11/Direct3D11.h>
#include <core/graphics/Direct3D11/BulletDebugDraw.h>
#include <core/graphics/Direct3D11/Effect.h>
#include <core/graphics/Shader.h>
#include <core/graphics/TextRenderer.h>
#include <core/graphics/OculusRift.h>


#include <core/sound/DirectSound/SoundEngine.h>
#include <core/sound/PortAudio/SoundEngine.h>

#include <core/sound/SoundManager.h>
#include <core/sound/Sound.h>

#include <core/input/DirectInput/DirectInput.h>
#include <core/physics/PhysicsManager.h>

#include <core/Logger.h>

#include <win/Version.h>
#include <win/Clipboard.h>

#include <game/Config.h>

#include <blue_sky/GameSettings.h>
#include <blue_sky/DebugLog.h>
#include "Scene/OverlayRenderRange.h"
#include "Scene/InputRouting.h"
#include <core/setting/SettingStore.h>
#include <core/ui/StringTable.h>
#include <core/ui/Theme.h>
#include <game/MainLoop.h>

#include <common/math.h>
#include <common/log.h>
#include <common/string.h>

#include <algorithm>
#include <filesystem>
#include <sstream>

#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/operations.hpp>

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

//■コンストラクタ
GameMain::GameMain( App& app )
	: app_( app )
	, dev_options_( dev::DevOptions::read_from_environment() )
	, scene_manager_( std::make_unique< SceneManager >() )
	, entity_manager_( std::make_unique< core::ecs::EntityManager >() )
	, is_display_fps_( false )
	, is_command_mode_( false )
	, is_show_cursor_( false )
	, user_command_( 1024, '\0' )
{
	// common::log( "log/debug.log", "init" );

	boost::filesystem::create_directory( "log" );

	win::Version version;
	version.log( "log/windows_version.log" );

	// get_app()->clip_cursor( true );

	create_scene_stack();

	save_data_.reset( new Config() );
	save_data_->set_log_handler( & debug_log );
	save_data_->load_file( "save/blue-sky.save" );

	// Direct3D
	direct_3d_.reset( new Direct3D(
		get_app()->GetWindowHandle(),
		get_app()->get_width(),
		get_app()->get_height(),
		get_config()->get( "graphics.multisample.count", 4 ), 
		get_config()->get( "graphics.multisample.quality", 2 )
	) );

	direct_3d_->get_effect()->load( "media/shader/main.fx" );

	direct_3d_->create_default_input_layout();

	// シェーダーの読み直し（開発中に .hlsl を保存したとき）
	// Effect は成功してから差し替え、取得済みの technique は中身だけ入れ替えるので、描画側のポインタはそのまま使える。
	// コンパイルに失敗したら、ログを出して前のシェーダーのまま続ける。
	// 監視は開発ビルド（_DEBUG）だけで行う。配布版ではシェーダーを書き換えないので、フォルダの監視と読み直しの経路を持たない
#ifdef _DEBUG
	get_app()->watch_directory_change( "media/shader/", [this] () {
		debug_log( "[Shader] media/shader/ の変更を検出したので読み直します" );

		try
		{
			direct_3d_->get_effect()->load( "media/shader/main.fx" );
		}
		catch ( const common::exception< std::string >& e )
		{
			debug_log( "[Shader] 読み直しに失敗しました（前のシェーダーのまま続けます）: " + e.data() );
			return;
		}
		catch ( const common::exception< HRESULT >& e )
		{
			debug_log( "[Shader] 読み直しに失敗しました（前のシェーダーのまま続けます）: HRESULT " + std::to_string( static_cast< unsigned long >( e.data() ) ) );
			return;
		}

		direct_3d_->create_default_input_layout();

		if ( graphics_manager_ )
		{
			graphics_manager_->refresh_all_shaders();
		}
	} );
#endif

	ImGui::CreateContext();
	ImGui_ImplWin32_Init( get_app()->GetWindowHandle() );
	ImGui_ImplDX11_Init( direct_3d_->getDevice(), direct_3d_->getImmediateContext() );

	ImGuiIO& io = ImGui::GetIO();
	// io.Fonts->AddFontFromFileTTF( "media/font/uzura.ttf", 22 );
	io.Fonts->AddFontFromFileTTF( "media/font/rounded-mplus-1p-regular.ttf", 28 );


	direct_input_.reset( new DirectInput( get_app()->GetInstanceHandle(), get_app()->GetWindowHandle() ) );

	input_.reset( new Input( get_app() ) );
	input_->set_direct_input( direct_input_.get() );
	input_->load_config( * get_config() );

	// Oculus Rift
	if ( get_config()->get( "input.oculus_rift.enabled", 0 ) )
	{
		oculus_rift_.reset( new OculusRift( direct_3d_.get() ) );
	}

	bullet_debug_draw_.reset( new core::graphics::direct_3d_11::BulletDebugDraw( direct_3d_.get() ) );
	bullet_debug_draw_->setDebugMode( get_config()->get< int >( "graphics.debug_bullet", 0 ) );

	scene_manager_->register_all_scene();

	// 物理の世界は本編と ECS のテストシーンで 1 つ（決定 1。以前は本編だけの ActiveObjectPhysics があり、デバッグ描画はそちらに設定していた）
	physics_manager_ = std::make_unique< core::physics::PhysicsManager >();
	physics_manager_->set_debug_drawer( bullet_debug_draw_.get() );

	graphics_manager_.reset( new blue_sky::graphics::direct_3d_11::GraphicsManager( direct_3d_.get() ) );
	graphics_manager_->set_debug_axis_enabled( get_config()->get< int >( "graphics.debug_axis", 0 ) != 0 );

	// 代わりのフォント（主のフォントに無い文字に使う。; 区切りで複数）
	if ( auto* text = graphics_manager_->get_text_renderer() )
	{
		std::stringstream fallbacks( static_cast< const Config& >( * get_config() ).get< string_t >( "graphics.font_fallback", "" ) );

		for ( string_t path; std::getline( fallbacks, path, ';' ); )
		{
			if ( ! path.empty() && ! text->add_fallback_font( path.c_str() ) )
			{
				debug_log( "[Graphics] 代わりのフォントを読み込めませんでした: " + path );
			}
		}
	}
	static_cast< graphics::direct_3d_11::GraphicsManager* >( graphics_manager_.get() )->set_bullet_debug_draw( bullet_debug_draw_.get() );

	// sound_engine_.reset( new core::sound::direct_sound::SoundEngine( get_app()->GetWindowHandle() ) );
	sound_engine_.reset( new core::sound::port_audio::SoundEngine() );
	sound_manager_.reset( new SoundManager( sound_engine_.get() ) );
	sound_manager_->start_update_thread();

	sound_manager_->load( "ok" );
	sound_manager_->load( "cancel" );
	sound_manager_->load( "click" );

	// 描画のフレームの定数に入れる経過時間（以前は core::Service に登録し、各所がそこから取っていた。設計上の問題 B28）
	graphics_manager_->set_time_manager( & time_manager_ );

	script_manager_.reset( new ScriptManager() );
	script_manager_->load_command_history( "log/script.log" );
	setup_script_command();

	// ゲームのオブジェクトに渡すもの（物理の世界・音・シーンの経過時間）。オブジェクトは GameMain のシングルトンを呼ばない
	game_object_context_.physics = physics_manager_.get();
	game_object_context_.sound_manager = sound_manager_.get();
	game_object_context_.scene_elapsed_time = [this] { const Scene* s = scene_stack_ ? scene_stack_->get_base_scene() : nullptr; return s ? s->get_total_elapsed_time() : 0.f; };
	game_object_context_.delta_time = [this] { return time_manager_.get_delta_time(); };

	object_updater_ = std::make_unique< ObjectUpdater >( physics_manager_.get(), & game_object_context_, entity_manager_.get() );
	object_factory_ = std::make_unique< ObjectFactory >( entity_manager_.get(), physics_manager_.get(), graphics_manager_.get(), sound_manager_.get() );

	// GraphicsManager はゲームの物体を、EntityManager の描画の部品（ecs::ObjectRenderDataSystem の描く対象）から描く（以前は ActiveObjectManager を直接回していた）
	graphics_manager_->set_entity_manager( entity_manager_.get() );

	// MainLoop
	main_loop_.reset( new MainLoop( 60 ) );

	// 開発用 : BLUE_SKY_DEBUG_FIXED_STEP_MSEC="ミリ秒" を設定すると、1 フレームの経過時間をいつもその値にする
	// （本当の時間によらず、物理・紙の模様・線の揺れがフレームの数だけで決まる。画面を撮り比べるときに、exe の速さの違いで揺れないように）
	if ( const unsigned long msec = dev_options_.fixed_step_msec; msec > 0 )
	{
		main_loop_->set_fixed_elapsed_msec( msec );
		debug_log( "[Debug] BLUE_SKY_DEBUG_FIXED_STEP_MSEC : 1 フレームの経過時間をいつも " + std::to_string( msec ) + " ミリ秒にします" );
	}

	// ゲーム設定 : ユーザーの設定ファイルから読み、表示系以外を反映する（表示系は App / Direct3D の初期化で、同じ登録を使って反映済み）
	settings_ = std::make_unique< core::setting::SettingStore >();
	settings_->set_error_handler( [] ( const string_t& message ) { debug_log( "[Setting] " + message ); } );
	game_settings::register_all( * settings_, * this );
	settings_->load( * get_app()->get_user_config() );
	game_settings::fix_invalid_values( * settings_ );
	settings_->apply_all_individual();

	if ( get_app()->is_background() )
	{
		debug_log( string_t( "[開発用] 裏で動かすので音を出しません（消音 : " ) + ( get_sound_manager()->is_mute() ? "はい" : "いいえ" ) + "）" );
	}

	// ユーザーの設定は settings.config が正。2 つのファイルに同じ項目が二重に残らないようにする
	// （どちらを編集すれば効くのか分からなくならないように）
	// - 開発用の blue-sky.config に残っている、ストアの項目は消す（以前の版から引き継いだもの）
	// - settings.config には、ストアの項目と版だけを残す（引き継ぎで写った開発用の項目は消す）
	{
		auto* user_config = get_app()->get_user_config();
		std::vector< string_t > not_user_settings;

		for ( const auto& [ name, value ] : user_config->get_values() )
		{
			if ( settings_->has( name ) )
			{
				get_config()->erase( name.c_str() );
			}
			else if ( name != core::setting::SettingStore::VERSION_KEY )
			{
				not_user_settings.push_back( name );
			}
		}

		for ( const auto& name : not_user_settings )
		{
			user_config->erase( name.c_str() );
		}
	}

	save_settings();

	// UI
	ui_theme_ = std::make_unique< core::ui::Theme >();
	ui_strings_ = std::make_unique< core::ui::StringTable >();
	ui_strings_->load_file( "media/text/ja.txt" );

	// 見た目の差し替え : 設定 ui.theme にテーマファイルが指定されていれば、既定のテーマを上書きする
	// （const で読む。非 const の get は既定値を設定に書き込んでしまうため）
	if ( const string_t theme_path = static_cast< const Config& >( * get_config() ).get< string_t >( "ui.theme", "" ); ! theme_path.empty() )
	{
		std::vector< string_t > errors;

		if ( ! core::ui::load_theme_file( * ui_theme_, theme_path.c_str(), & errors ) )
		{
			debug_log( "[UI] テーマファイルを開けません: " + theme_path );
		}

		for ( const auto& e : errors )
		{
			debug_log( "[UI] テーマファイル " + theme_path + " の " + e );
		}
	}

	// 文字の大きさの設定は、テーマファイルで決めた大きさを基準に掛ける
	ui_theme_base_ = std::make_unique< core::ui::Theme >( * ui_theme_ );
	set_ui_text_scale( ui_text_scale_ );

	update_render_data_for_game();
}

//■デストラクタ
GameMain::~GameMain()
{
	// シーン（SceneStack）を最初に壊す。シーンは設定（購読）・文字列表・テーマ・ECS など GameMain の持ち物を使う。
	// scene_stack_ は最後に宣言しているので放っておいても最初に壊れるが、決まりとしてここで壊す
	// （以前はゲーム本編を開いたまま閉じると、GamePlayScene が壊れた SettingStore から購読を外そうとして毎回落ちていた）
	scene_stack_.reset();

	// ゲームの物体を EntityManager より先に壊す（物体は壊れるときに自分の Entity を消す。entity_manager_ は object_factory_ より後に宣言され、先に壊れる）。
	// ECS だけの物体（ObjectFactory が作った）も一緒に消す
	if ( object_factory_ )
	{
		object_factory_->clear();
	}

	ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

	// 設定の保存は shutdown() で行う（正常終了のときだけ）。デストラクタは例外で抜けた経路でも動くので、ここでは保存しない
}

/**
 * 正常終了の後片付け（メッセージループが正常に終わったときだけ呼ばれる）
 *
 * 以前は static の破棄（WinMain が戻った後）で保存していたため、例外で抜けたときにも壊れた状態を保存し得た。
 */
void GameMain::shutdown()
{
	if ( is_shut_down_ )
	{
		return;
	}

	is_shut_down_ = true;

	// 翻訳漏れ（引かれたが文字列テーブルに無かったキー）をログに出す
	for ( const auto& key : ui_strings_->get_missing_keys() )
	{
		debug_log( "[UI] 文字列テーブルに無いキー: " + key );
	}

	// 設定を保存する。ユーザーの明示的な操作（F2 / F5 / Alt+Enter）はその場で設定に反映済み。
	// 終了時の実際の状態（フォーカス喪失で解除されたフルスクリーンなど）は取り込まない
	save_settings();

	if ( ! get_config()->save_file( "blue-sky.config" ) )
	{
		debug_log( "[Config] 開発用の設定ファイルを保存できませんでした: blue-sky.config" );
	}

	get_script_manager()->save_command_history( "log/script.log" );
}

unsigned long GameMain::get_idle_wait_msec() const
{
	return main_loop_ ? main_loop_->get_wait_msec() : 0;
}

void GameMain::set_ui_text_scale( float_t scale )
{
	ui_text_scale_ = scale;

	// 設定の反映はテーマを作る前にも呼ばれる（起動時）。テーマができたら apply_ui_text_scale() で反映する
	if ( ui_theme_ && ui_theme_base_ )
	{
		const uint_t revision = ui_theme_->revision;
		*ui_theme_ = core::ui::scale_theme_text( *ui_theme_base_, scale );
		ui_theme_->revision = revision + 1;		// 開いている画面が配置し直すきっかけ
	}
}

void GameMain::save_settings()
{
	settings_->save( * get_app()->get_user_config(), game_settings::USER_SETTINGS_VERSION );

	const string_t user_settings_path = game_settings::user_settings_path();

	// 置き場所のフォルダが無ければ作る（初回）
	std::error_code ec;
	std::filesystem::create_directories( std::filesystem::path( user_settings_path ).parent_path(), ec );

	if ( ! get_app()->get_user_config()->save_file( user_settings_path.c_str() ) )
	{
		debug_log( string_t( "[Config] ユーザーの設定ファイルを保存できませんでした: " ) + user_settings_path );
	}
}

/**
 * ゲーム全体で共有可能なスクリプトコマンドを準備する
 *
 */
void GameMain::setup_script_command()
{
	get_script_manager()->exec( "function load( s ) dofile( 'media/script/' .. s ) end" );

	// Scene
	// 遷移はその場で実行せず、要求として積む（スクリプトがシーンの更新中に呼ばれても、実行中のシーンを破棄しないように）
	// 名前の後に "?キー=値" で引数を付けられる（例 : push_overlay( "options?tab=controls" )）
	get_script_manager()->set_function( "scene", [this] ( const char_t* name ) { SceneRequest r { SceneRequest::Type::REPLACE_BASE }; split_scene_name( name, r.name, r.args ); request_scene( r ); } );
	get_script_manager()->set_function( "push_overlay", [this] ( const char_t* name ) { SceneRequest r { SceneRequest::Type::PUSH_OVERLAY }; split_scene_name( name, r.name, r.args ); request_scene( r ); } );
	get_script_manager()->set_function( "pop_overlay", [this] () { request_scene( SceneRequest{ SceneRequest::Type::POP_OVERLAY } ); } );

	// Basic
	get_script_manager()->set_function( "color", [this] ( float_t r, float_t g, float_t b, float_t a ) { return Color( r, g, b, a ); } );
	get_script_manager()->set_function( "vector", [this] ( float_t x, float_t y, float_t z ) { return Vector( x, y, z ); } );

	// Fade
	get_script_manager()->set_function( "set_fade_color", [this] ( const Color& color ) { get_graphics_manager()->set_fade_color( color ); } );
	get_script_manager()->set_function( "start_fade_in", [this] ( float_t speed ) { get_graphics_manager()->start_fade_in( speed ); } );
	get_script_manager()->set_function( "start_fade_out", [this] ( float_t speed ) { get_graphics_manager()->start_fade_out( speed ); } );

	// ゲームの物体（Lua の "GameObject" は EntityId を包んだ GameObjectRef。ActiveObject を持たない ECS だけの物体も同じように触れる）
	// 知らない名前なら nil を返す（以前は null に restart を呼んで落ちた）
	get_script_manager()->set_function( "create_object", [this] ( const char_t* name ) -> sol::optional< GameObjectRef >
	{
		auto id = get_object_factory()->create_object( name );

		if ( ! id )
		{
			return sol::nullopt;
		}

		GameObjectRef o = get_object_factory()->get_ref( * id );
		o.restart();

		return o;
	} );
	get_script_manager()->set_function( "clone_object", [this] ( const GameObjectRef* o ) -> sol::optional< GameObjectRef >
	{
		// 以前は ActiveObject の種類だけ複製できた（段階 6 で最後の種類のプレイヤーも ECS だけの物体になり、複製できる物体は無くなった）。
		// ECS だけの物体は create_object で作り直す。黙って無視しない
		require_not_nil( o, "clone_object" );
		debug_log( "[Object] clone_object : 物体は複製できません（create_object で作り直してください）" );

		return sol::nullopt;
	} );
	get_script_manager()->set_function( "get_object", [this] ( const char_t* name ) -> sol::optional< GameObjectRef >
	{
		if ( auto o = get_object_factory()->find_ref( name ) )
		{
			return * o;
		}

		return sol::nullopt;
	} );
	get_script_manager()->set_function( "set_name", [this] ( const GameObjectRef* o, const char_t* name ) { get_object_factory()->name( name, require_not_nil( o, "set_name" )->get_entity_id() ); } );
	get_script_manager()->set_function( "set_loc", [] ( GameObjectRef* o, float_t x, float_t y, float_t z ) { require_not_nil( o, "set_loc" )->set_location( x, y, z ); } );
	get_script_manager()->set_function( "set_dir", [] ( GameObjectRef* o, float_t r ) { require_not_nil( o, "set_dir" )->set_direction_degree( r ); } );
	get_script_manager()->set_function( "set_vel", [] ( GameObjectRef* o, float_t x, float_t y, float_t z ) { require_not_nil( o, "set_vel" )->set_velocity( Vector( x, y, z ) ); } );
	get_script_manager()->set_function( "set_start_loc", [] ( GameObjectRef* o, float_t  x, float_t  y, float_t  z ) { require_not_nil( o, "set_start_loc" )->set_start_location( x, y, z ); } );
	get_script_manager()->set_function( "set_start_rot", [] ( GameObjectRef* o, float_t rx, float_t ry, float_t rz ) { require_not_nil( o, "set_start_rot" )->set_start_rotation( rx, ry, rz ); } );
	get_script_manager()->set_function( "set_start_dir", [] ( GameObjectRef* o, float_t r ) { require_not_nil( o, "set_start_dir" )->set_start_direction_degree( r ); } );
	get_script_manager()->set_function( "set_mass", [] ( GameObjectRef* o, float_t mass ) { require_not_nil( o, "set_mass" )->set_mass( mass ); } );
	get_script_manager()->set_function( "make_model_unique", [] ( GameObjectRef* o ) { return require_not_nil( o, "make_model_unique" )->make_model_unique(); } );

	// GrahpicsManager
	get_script_manager()->set_function( "setup_shadow_map", [this] ( uint_t level = 3, uint_t size = 1024 ) { get_graphics_manager()->setup_shadow_map( level, size ); } );
	get_script_manager()->set_function( "set_sky_box", [this] ( const char_t* name ) { get_graphics_manager()->set_sky_box( name ); } );
	get_script_manager()->set_function( "unset_sky_box", [this] () { get_graphics_manager()->unset_sky_box(); } );
	get_script_manager()->set_function( "set_ground", [this] ( const char_t* name ) { get_graphics_manager()->set_ground( name ); } );
	get_script_manager()->set_function( "unset_ground", [this] () { get_graphics_manager()->unset_ground(); } );

	get_script_manager()->set_function( "set_paper_texture_type", [this] ( int_t type ) { get_graphics_manager()->set_paper_texture_type( type ); } );
	get_script_manager()->set_function( "set_ambient_color", [this] ( const Color& color ) { get_graphics_manager()->set_ambient_color( color ); } );
	get_script_manager()->set_function( "set_shadow_color", [this] ( const Color& color ) { get_graphics_manager()->set_shadow_color( color ); } );
	get_script_manager()->set_function( "set_shadow_paper_color", [this] ( const Color& color ) { get_graphics_manager()->set_shadow_paper_color( color ); } );
	get_script_manager()->set_function( "set_drawing_accent", [this] ( float_t accent ) { get_graphics_manager()->set_drawing_accent( accent ); } );
	get_script_manager()->set_function( "set_drawing_line_type", [this] ( int_t type ) { get_graphics_manager()->set_drawing_line_type( type ); } );
	get_script_manager()->set_function( "set_light_direction", [this] ( float_t x, float_t y, float_t z ) { return get_graphics_manager()->set_light_direction( Vector( x, y, z, 0.f ) ); } );

	get_script_manager()->set_function( "set_post_effect_shader", [this] ( core::graphics::Shader* s ) { get_graphics_manager()->set_post_effect_shader( require_not_nil( s, "set_post_effect_shader" ) ); } );

	get_script_manager()->set_function( "load_model", [this] ( const char_t* name ) { return get_graphics_manager()->load_model( name ); } );
	get_script_manager()->set_function( "clone_model", [this] ( graphics::Model* m ) { auto* m2 = get_graphics_manager()->clone_model( require_not_nil( m, "clone_model" ) ); return m2; } );
	get_script_manager()->set_function( "clone_model_with_shaders", [this] ( graphics::Model* m ) { return get_graphics_manager()->clone_model_with_shaders( require_not_nil( m, "clone_model_with_shaders" ) ); } );

	get_script_manager()->set_function( "get_shader", [this] ( const char_t* name ) { return get_graphics_manager()->get_shader( name ); } );
	get_script_manager()->set_function( "clone_shader", [this] ( const core::graphics::Shader* s ) { return get_graphics_manager()->clone_shader( require_not_nil( s, "clone_shader" ) ); } );

	get_script_manager()->set_function( "load_texture", [this] ( const char_t* file_path ) { return get_graphics_manager()->load_texture( file_path ); } );
	// get_script_manager()->set_function( "load_named_texture", [this] ( const char_t* name, const char_t* file_path ) { return get_graphics_manager()->load_texture( name, file_path ); } );
	get_script_manager()->set_function( "get_texture", [this] ( const char_t* name ) { return get_graphics_manager()->get_texture( name ); } );

	// SoundManager
	get_script_manager()->set_function( "play_sound", [this] ( const char_t* name ) { if ( auto* s = get_sound_manager()->get_sound( name ) ) { s->play( false ); } } );

	// debug
	get_script_manager()->set_function( "debug_axis", [this] ( int on ) { get_graphics_manager()->set_debug_axis_enabled( on != 0 ); } );
	get_script_manager()->set_function( "debug_bullet", [this] ( int mode ) { bullet_debug_draw_->setDebugMode( mode ); } );

	get_script_manager()->set_function( "debug_print_resources", [this] () { get_graphics_manager()->debug_print_resources(); } );

}

/**
 * ゲーム毎に更新する必要のある描画用の定数バッファを更新する
 *
 */
void GameMain::update_render_data_for_game() const
{
	GameConstantBufferData constant_buffer_data = {
		static_cast< float_t >( get_width() ),
		static_cast< float_t >( get_height() )
	};
		
	get_graphics_manager()->get_game_render_data()->update( & constant_buffer_data );
}

bool GameMain::update()
{
	if ( ! main_loop_->loop() )
	{
		return false;
	}

	time_manager_.update( main_loop_->get_elapsed_sec() );

	if ( frame_time_last_ > 0 )
	{
		LARGE_INTEGER now;
		QueryPerformanceCounter( & now );
		frame_time_previous_start_ = frame_time_start_;
		frame_time_start_ = now.QuadPart;
	}

	// 監視しているディレクトリ（シェーダーなど）の変更を毎フレーム確かめる
	// （以前はウィンドウが再びアクティブになったときだけ確かめていたので、エディタと並べて作業していると読み直されなかった。
	//   待ち時間 0 の確認なので負荷はほぼない）
	get_app()->check_directory_change();

	// 前のフレームでベースシーンの置き換えが決まっていたら、「読み込み中」を表示してから置き換える
	if ( scene_stack_->is_loading_frame() )
	{
		scene_stack_->replace_base_with_loading_screen();
		return true;
	}

	// サウンドの更新は SoundManager の更新スレッドが行う（メインスレッドが止まっても BGM が途切れないように）
	if ( ! get_sound_manager()->is_update_thread_running() )
	{
		get_sound_manager()->update();
	}
	
	if ( get_app()->is_active() )
	{
		direct_input_->update();
		input_->update();

		if ( is_command_mode_ )
		{
			input_->clear_mouse_move();
		}
	}
	else
	{
		input_->update_null();
	}

	if ( oculus_rift_ )
	{
		oculus_rift_->update();
	}

	handle_escape();

	ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();

	// imgui は描画の解像度で描くので、大きさとマウスの位置を描画の座標に直す（フルスクリーン（ボーダーレス）ではウィンドウが描画より大きい）
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2( static_cast< float >( get_app()->get_width() ), static_cast< float >( get_app()->get_height() ) );

		POINT cursor;

		if ( get_app()->is_full_screen() && GetCursorPos( & cursor ) && ScreenToClient( get_app()->GetWindowHandle(), & cursor ) )
		{
			cursor = get_app()->client_to_render( cursor );
			io.AddMousePosEvent( static_cast< float >( cursor.x ), static_cast< float >( cursor.y ) );
		}
	}

    ImGui::NewFrame();

	// 別スレッドの読み込みが終わったものを、メインスレッドで処理する（テクスチャの作成など）
	graphics_manager_->dispatch_async_loads();

	// 入力を受け取る先を 1 か所で決める（コンソール → オーバーレイ → ベースシーン）。
	// コンソールで打った文字やマウス操作で、UI やキャラクターが動かないように
	const ImGuiIO& io = ImGui::GetIO();
	const InputOwner input_owner = decide_input_owner( io.WantCaptureKeyboard || io.WantCaptureMouse, has_overlay_scene() );

	if ( has_overlay_scene() )
	{
		// ベースシーンはポーズ中 (更新しない)。オーバーレイシーンのトップのみ更新
		input_->set_blocked( input_owner != InputOwner::OVERLAY );
		scene_stack_->get_top_overlay()->update();
	}
	else
	{
		input_->set_blocked( input_owner != InputOwner::BASE_SCENE );
		scene_stack_->get_base_scene()->update();
	}

	input_->set_blocked( false );

	render();

	// 開発用 : 起動時のオーバーレイ（数フレーム後に開く）
	scene_stack_->update_start_overlay();

	// 開発用 : BLUE_SKY_DEBUG_SCENE_SEQUENCE の順に、150 フレームごとにベースシーンを置き換える
	if ( ! debug_scene_sequence_.empty() && ! scene_stack_->is_loading_frame() && ++debug_scene_sequence_frames_ >= 150 )
	{
		debug_scene_sequence_frames_ = 0;
		debug_log( "[Scene] 開発用の順でベースシーンを置き換えます: " + debug_scene_sequence_.front().name + ( debug_scene_sequence_.front().stage_name.empty() ? "" : " " + debug_scene_sequence_.front().stage_name ) );
		request_scene( debug_scene_sequence_.front() );
		debug_scene_sequence_.pop_front();
	}

	// シーン遷移 : シーンが出した要求を集め、フレームの境目でまとめて実行する
	scene_stack_->collect_requests();
	scene_stack_->process_requests();

	// 表示の変更は、UI のコールバックやウィンドウメッセージの処理中ではなく、ここでまとめて行う
	//
	// 設定（ユーザーが選んだ値）を書き換えるのはユーザーの明示的な操作（オプション画面・Alt+Enter・F5）だけ。
	// フルスクリーンはボーダーレスなので、フォーカスを失っても解除されない（以前の排他フルスクリーンは解除され、戻す処理が要った）
	if ( is_full_screen_toggle_requested_ )
	{
		is_full_screen_toggle_requested_ = false;

		get_app()->set_full_screen( ! get_app()->is_full_screen() );
		settings_->sync_from_runtime( game_settings::FULL_SCREEN, get_app()->is_full_screen() );
	}

	if ( is_display_change_requested_ )
	{
		is_display_change_requested_ = false;
		apply_display_settings();
	}

	return true;
}

/**
 * 設定ストアの確定値に合わせて、解像度・フルスクリーンを変更する
 *
 * フルスクリーンはボーダーレス（画面いっぱいの縁なしウィンドウ）なので、解像度はフルスクリーンのままでも変えられる
 * （描画の解像度が変わり、表示のときに画面の大きさへ拡大される。以前の排他フルスクリーンは、いったんウィンドウに戻す必要があった）
 */
void GameMain::apply_display_settings()
{
	const int w = settings_->get_committed( game_settings::SCREEN_WIDTH );
	const int h = settings_->get_committed( game_settings::SCREEN_HEIGHT );
	const bool full_screen = settings_->get_committed( game_settings::FULL_SCREEN );

	if ( w != get_app()->get_width() || h != get_app()->get_height() )
	{
		get_app()->set_size( w, h );
	}

	get_app()->set_full_screen( full_screen );

	// フルスクリーンの状態は、以前は 3 か所（App・スワップチェーン・設定）が持ち、毎回手で写していた。
	// 今は役割を分ける : 実際の状態は App（ウィンドウ）だけ、ユーザーの選択は設定だけ

	debug_log( "[Display] 表示の設定を適用しました : " + std::to_string( get_app()->get_width() ) + "x" + std::to_string( get_app()->get_height() )
		+ "・描画先 " + std::to_string( direct_3d_->get_width() ) + "x" + std::to_string( direct_3d_->get_height() )
		+ "・フルスクリーン " + ( get_app()->is_full_screen() ? "はい" : "いいえ" ) );
}

/**
 * 開発用のフレームの保存を準備する
 *
 * 環境変数 BLUE_SKY_CAPTURE_FRAMES=60,120（保存するフレームの番号）と BLUE_SKY_CAPTURE_DIR（既定 dump）。
 * 自動操作の確認で、画面がロック中（ウィンドウを撮影できない）でも描画結果を確かめるために使う。
 */
void GameMain::setup_frame_capture()
{
	if ( dev_options_.capture_frames.empty() )
	{
		return;
	}

	capture_frames_ = dev_options_.capture_frames;
	capture_directory_ = dev_options_.capture_directory;
	CreateDirectoryA( capture_directory_.c_str(), nullptr );
}

void GameMain::capture_frame_if_requested()
{
	frame_count_++;

	// 開発用 : BLUE_SKY_DEBUG_DISPLAY_CHANGE（"幅x高さ"）を設定すると、120 フレーム目に解像度を変えて「適用」する
	// （排他フルスクリーン中の解像度の変更を、オプション画面を操作せずに確かめるため。設定ファイルを汚さないよう BLUE_SKY_SETTINGS_DIR と一緒に使う）
	if ( frame_count_ == 120 && dev_options_.display_change_width > 0 && dev_options_.display_change_height > 0 )
	{
		const int w = dev_options_.display_change_width, h = dev_options_.display_change_height;

		debug_log( "[Debug] 解像度を変えて適用します : " + std::to_string( w ) + "x" + std::to_string( h ) );
		settings_->set( game_settings::SCREEN_WIDTH, w );
		settings_->set( game_settings::SCREEN_HEIGHT, h );
		settings_->apply_deferred();
	}

	if ( std::find( capture_frames_.begin(), capture_frames_.end(), frame_count_ ) == capture_frames_.end() )
	{
		return;
	}

	wchar_t name[ 32 ];
	swprintf_s( name, L"/capture_%04d.png", frame_count_ );	// 区切りは / （Windows でも使える）

	const std::wstring path = common::convert_to_wstring( capture_directory_ ) + name;
	const bool ok = direct_3d_->save_back_buffer_to_png( path.c_str() );

	// 文字のアトラスも保存する（BLUE_SKY_CAPTURE_GLYPH_ATLAS が設定されているとき。文字の描画の不具合を調べる）
	if ( dev_options_.capture_glyph_atlas )
	{
		swprintf_s( name, L"/atlas_%04d.png", frame_count_ );
		graphics_manager_->save_glyph_atlas( ( common::convert_to_wstring( capture_directory_ ) + name ).c_str() );
	}

	debug_log( string_t( ok ? "フレームを保存しました: " : "フレームを保存できませんでした: " ) + capture_directory_ + " / " + std::to_string( frame_count_ ) );
}

void GameMain::render()
{
	// 描画する範囲を上から決める
	// - 不透明なオーバーレイ（is_transparent() が false）より下は、ベースシーンも含めて描かない
	// - 下のオーバーレイを隠すオーバーレイ（メニューなど）より下のオーバーレイは描かない（ベースシーンは描く）
	const auto& overlays = scene_stack_->get_overlays();
	std::vector< OverlayLayerInfo > layers;
	layers.reserve( overlays.size() );

	for ( const auto& overlay : overlays )
	{
		layers.push_back( { overlay->is_transparent(), overlay->hides_overlays_below() } );
	}

	const OverlayRenderRange range = compute_overlay_render_range( layers );

	// 前のフレームの描画状態を持ち越さない。画面と深度もここで必ずクリアする
	// （以前はクリアをベースシーンの render() に任せていて、クリアしないシーン（enemy_ai_test など）では
	//   前のフレームの深度が残り、深度テストのある描画が消えたり、残像が出たりした）
	direct_3d_->reset_render_state();
	get_graphics_manager()->clear_default_view();

	if ( range.render_base )
	{
		scene_stack_->get_base_scene()->render();
	}

	// オーバーレイシーンを下から順に描画
	for ( size_t i = range.first_overlay; i < overlays.size(); i++ )
	{
		// 下のシーン（ポストエフェクトなど）の状態を持ち越さない
		direct_3d_->reset_render_state();
		overlays[ i ]->render();
	}

	if ( is_command_mode_ )
    {
		render_console_window();
		render_scene_list_window();
	}

	ImGui::Render();
	get_graphics_manager()->set_default_render_target( false );
    ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData() );

	capture_frame_if_requested();
	record_frame_time();

	// 開発用 : present にかかった時間も測る（BLUE_SKY_DEBUG_FRAME_TIME の範囲。最後のフレームの分はログの後なので入らない）
	const bool measure_present = frame_time_last_ > 0 && frame_count_ >= frame_time_first_ && frame_count_ < frame_time_last_;
	LARGE_INTEGER present_start = { }, present_end = { };

	if ( measure_present )
	{
		QueryPerformanceCounter( & present_start );
	}

	direct_3d_->present();

	if ( measure_present )
	{
		LARGE_INTEGER frequency;
		QueryPerformanceCounter( & present_end );
		QueryPerformanceFrequency( & frequency );
		frame_time_present_ms_.push_back( static_cast< double >( present_end.QuadPart - present_start.QuadPart ) * 1000.0 / static_cast< double >( frequency.QuadPart ) );
	}

	// フレームの途中で解放・差し替えを頼まれたリソースを、ここでまとめて解放する（フレームの中では引いたポインタを使い回せる。doc/ハンドルの設計.md）
	flush_resource_releases();
}

/// 解放・差し替えを頼まれたリソースをまとめて解放する（フレームの終わりとシーンの切り替えで呼ぶ）
void GameMain::flush_resource_releases()
{
	graphics_manager_->flush_releases();
	sound_manager_->flush_releases();
}

/**
 * 開発用のフレームの時間の計測を準備する
 *
 * 環境変数 BLUE_SKY_DEBUG_FRAME_TIME="最初のフレーム,最後のフレーム"（例 "300,1500"）。
 * その間のフレームについて、update の始まりから present の前まで（CPU の仕事の時間。垂直同期の待ちを含まない）と、
 * フレームの間隔を測り、最後のフレームで中央値などを debug_log に出す（ハンドルの作り替えの前後を比べるため。doc/ハンドルの設計.md）。
 */
void GameMain::setup_frame_time_measurement()
{
	frame_time_first_ = dev_options_.frame_time_first;
	frame_time_last_ = dev_options_.frame_time_last;

	if ( frame_time_last_ <= 0 )
	{
		return;
	}

	frame_time_work_ms_.reserve( frame_time_last_ - frame_time_first_ + 1 );
	frame_time_interval_ms_.reserve( frame_time_last_ - frame_time_first_ + 1 );
	frame_time_present_ms_.reserve( frame_time_last_ - frame_time_first_ + 1 );
}

void GameMain::record_frame_time()
{
	if ( frame_time_last_ <= 0 || frame_count_ < frame_time_first_ || frame_count_ > frame_time_last_ || frame_time_start_ == 0 )
	{
		return;
	}

	LARGE_INTEGER now, frequency;
	QueryPerformanceCounter( & now );
	QueryPerformanceFrequency( & frequency );

	const double to_ms = 1000.0 / static_cast< double >( frequency.QuadPart );
	frame_time_work_ms_.push_back( static_cast< double >( now.QuadPart - frame_time_start_ ) * to_ms );

	if ( frame_time_previous_start_ != 0 )
	{
		frame_time_interval_ms_.push_back( static_cast< double >( frame_time_start_ - frame_time_previous_start_ ) * to_ms );
	}

	if ( frame_count_ != frame_time_last_ )
	{
		return;
	}

	// 中央値・平均・95 パーセンタイル・最小・最大
	auto summary = [] ( std::vector< double > v )
	{
		if ( v.empty() )
		{
			return string_t( "なし" );
		}

		std::sort( v.begin(), v.end() );
		double sum = 0.0;

		for ( double x : v )
		{
			sum += x;
		}

		char text[ 160 ];
		sprintf_s( text, "中央値 %.3f ms / 平均 %.3f ms / 95%% %.3f ms / 最小 %.3f ms / 最大 %.3f ms",
			v[ v.size() / 2 ], sum / v.size(), v[ std::min( v.size() - 1, v.size() * 95 / 100 ) ], v.front(), v.back() );
		return string_t( text );
	};

	debug_log( "[FrameTime] フレーム " + std::to_string( frame_time_first_ ) + "〜" + std::to_string( frame_time_last_ ) + "（" + std::to_string( frame_time_work_ms_.size() ) + " 枚）"
		+ " シーン " + ( get_current_scene() ? get_current_scene()->get_name() : string_t( "なし" ) ) );
	debug_log( "[FrameTime] 仕事の時間（update の始まり〜present の前） : " + summary( frame_time_work_ms_ ) );
	debug_log( "[FrameTime] フレームの間隔 : " + summary( frame_time_interval_ms_ ) );
	debug_log( "[FrameTime] present の時間 : " + summary( frame_time_present_ms_ ) );
}

/**
 * @brief コンソールウィンドウを描画する
 */
void GameMain::render_console_window()
{
	if ( ! ImGui::Begin( "Console", 0 ) )
    {
		ImGui::End();
		return;
	}

	static auto scroll_to_bottom = false;

	const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
	if ( ImGui::BeginChild( "ScrollingRegion", ImVec2( 0, -footer_height_to_reserve ), 0, ImGuiWindowFlags_HorizontalScrollbar ) )
    {
		if ( ImGui::BeginPopupContextWindow() )
        {
            if ( ImGui::Selectable( "Clear" ) )
			{
				// clear log
			}

            ImGui::EndPopup();
        }

		for ( const auto& log: core::logger.get_log_list() )
		{
			auto has_color = false;

			switch ( log.get_type() )
			{
			case core::Logger::Log::Type::ERROR:
				ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 0.4f, 0.4f, 1.0f ) );
				has_color = true;
				break;
			case core::Logger::Log::Type::WARN:
				ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 0.7f, 0.1f, 1.0f ) );
				has_color = true;
				break;
			case core::Logger::Log::Type::INFO:
				break;
			case core::Logger::Log::Type::DEBUG:
				break;
			}
                    

			ImGui::TextUnformatted( log.get_message().c_str() );

			if ( has_color )
			{
				ImGui::PopStyleColor();
			}
		}

		if ( scroll_to_bottom )
		{
            ImGui::SetScrollHereY( 1.f );
		}

        scroll_to_bottom = false;
	}

	ImGui::EndChild();

	ImGui::Separator();

	const auto input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;

	ImGui::SetNextItemWidth( -FLT_MIN );

	bool reclaim_focus = false;

	const auto callback = [] (ImGuiInputTextCallbackData* data) -> int
	{
		auto game_main = static_cast< GameMain* >( data->UserData );
		string_t command;

		switch ( data->EventFlag )
		{
		case ImGuiInputTextFlags_CallbackCompletion:
			command = game_main->user_command_;

			game_main->get_script_manager()->auto_complete( command );

			if ( ! game_main->get_script_manager()->get_output().empty() )
			{
				core::logger.info( game_main->get_script_manager()->get_output() );
			}

			scroll_to_bottom = true;

			break;
		case ImGuiInputTextFlags_CallbackHistory:
			if ( data->EventKey == ImGuiKey_UpArrow )
			{
				command = game_main->get_script_manager()->get_prev_hisotry_command();
			
			}
			else if ( data->EventKey == ImGuiKey_DownArrow )
			{
				command = game_main->get_script_manager()->get_next_hisotry_command();	
			}
		}

		if ( command != game_main->user_command_ )
		{
			data->DeleteChars( 0, data->BufTextLen );
			data->InsertChars( 0, command.c_str() );
		}

		return 0;
	};

	if ( ImGui::InputText( "##command", & user_command_, input_text_flags, callback, this ) )
    {
		boost::trim( user_command_ );

		if ( user_command_ != "" )
		{
			core::logger.info( user_command_ );

			try
			{
				get_script_manager()->exec( user_command_, true );
			}
			catch ( const ScriptError& e )
			{
				// get_app()->show_error_message( e.what() );
				core::logger.error( e.what() );
			}

			user_command_ = "";
			scroll_to_bottom = true;
		}

		reclaim_focus = true;
	}

	ImGui::SetItemDefaultFocus();

    if ( reclaim_focus || ! ImGui::IsAnyItemActive() )
	{
		ImGui::SetKeyboardFocusHere( -1 );
	}

	ImGui::End();
}
/**
 * @brief シーン一覧ウィンドウを描画する
 */
void GameMain::render_scene_list_window()
{
	if ( ! ImGui::Begin( "SceneList" ) )
    {
		ImGui::End();
		return;
	}

	if ( ImGui::BeginListBox( "##SceneListBox", ImVec2( -FLT_MIN, 0 ) ) )
    {
		static int selected_item_index = -1;

		int n = 0;

        for ( const auto& scene: scene_manager_->get_scene_generator_map() )
        {
            ImGuiSelectableFlags flags = n == selected_item_index ? ImGuiSelectableFlags_Highlight : 0;

            if ( ImGui::Selectable( scene.first.c_str(), n == selected_item_index, flags ) )
			{
				selected_item_index = n;
			}

			if ( ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked( 0 ) )
			{
				scene_stack_->setup_base_scene( scene.first );
			}

			n++;
        }
        
		ImGui::EndListBox();
    }

	ImGui::End();
}

void GameMain::on_key_down( char_t c )
{
	// 文字の入力（WM_CHAR）。UI の文字入力（TextField）が取り出す
	input_->push_char_byte( c );
}

void GameMain::set_vsync( bool v )
{
	direct_3d_->set_vsync( v );
}

void GameMain::on_raw_key_down( int vk )
{
	input_->latch_key_press( vk );
}

void GameMain::on_raw_mouse_move( long dx, long dy )
{
	input_->add_raw_mouse_move( dx, dy );
}

void GameMain::on_ime_composition( const std::u32string& text )
{
	input_->set_composition( text );
}

void GameMain::on_special_key_down( int key )
{
	// キー割り当てで押されたキーを待っている間は、切り替えを動かさない（以前は F2 でミュート・F5 でフルスクリーンが切り替わった）
	if ( const Scene* top = scene_stack_->get_top_overlay(); top && top->is_capturing_key() )
	{
		return;
	}

	if ( key == VK_F1 )
	{
		is_command_mode_ = ! is_command_mode_;
		update_cursor_state();
	}
	else if ( key == VK_F2 )
	{
		get_sound_manager()->set_mute( ! get_sound_manager()->is_mute() || get_app()->is_background() );	// 裏で動かすときは鳴らさない
		settings_->sync_from_runtime( game_settings::AUDIO_MUTE, get_sound_manager()->is_mute() );
		core::Sound* bgm = get_sound_manager()->get_sound( "bgm" );

		if ( bgm )
		{
			bgm->play( true );
		}
	}
	else if ( key == VK_F5 )
	{
		on_toggle_full_screen();
	}

	// F1〜F12 だけを番号（1〜12）にしてシーンに渡す（以前はどのキーも key - F1 + 1 にして渡し、文字のキーでも呼ばれていた）
	if ( key >= VK_F1 && key <= VK_F12 )
	{
		scene_stack_->get_base_scene()->on_function_key_down( key - VK_F1 + 1 );
	}
}

void GameMain::on_mouse_wheel( int wheel )
{
	input_->push_mouse_wheel_queue( wheel > 0 ? 1 : -1 );
}

void GameMain::on_resize()
{
	if ( direct_3d_ )
	{
		direct_3d_->on_resize( get_app()->get_width(), get_app()->get_height() );
		update_render_data_for_game();
	}
}

void GameMain::handle_escape()
{
	// ESC キーはオーバーレイの有無に関わらず検出する（ブロックをバイパス）
	const bool escape_blocked = input_->is_blocked();
	input_->set_blocked( false );
	const bool escape_pushed = input_->push( Input::Button::ESCAPE );
	input_->set_blocked( escape_blocked );

	// 開発用のコンソール（ImGui）がキー入力を受け取っているときは、ESC はコンソールのもの。
	// 何をするか（オーバーレイが開いていれば UI に任せる・終了・ポーズメニュー・タイトルへ）は SceneStack が決める
	scene_stack_->handle_escape( escape_pushed && ! ImGui::GetIO().WantCaptureKeyboard );
}

/**
 * シーンの持ち物（SceneStack）を作る
 *
 * シーンを作る・片付ける・画面に出すなどの GameMain の仕事を渡す（SceneStack は GameMain を知らない。テストでは偽物を渡す）
 */
void GameMain::create_scene_stack()
{
	SceneStack::Hooks hooks;

	hooks.is_registered = [this] ( const string_t& name ) { return scene_manager_->is_scene_registered( name ); };

	// 引数はシーンのコンストラクタが get_next_scene_arguments（SceneStack が作っている間だけ見せる）から受け取る
	hooks.create_scene = [this] ( const string_t& name, const SceneArguments& ) { return std::unique_ptr< Scene >( scene_manager_->generate_scene( name, * this ) ); };

	hooks.before_base_destroyed = [this] ( const string_t& name )
	{
		sound_manager_->pop_group();
		sound_manager_->push_group( name.c_str() );
	};

	hooks.after_base_destroyed = [this] { clear_for_next_scene(); };

	hooks.show_loading_screen = [this]
	{
		get_graphics_manager()->set_default_render_target( true );
		get_graphics_manager()->clear_default_view();
		get_graphics_manager()->draw_text_at_center( get_ui_strings().get( "loading" ).c_str(), Color( 0.8f, 0.8f, 0.8f, 1.f ) );
		direct_3d_->present();
	};

	hooks.after_base_replaced = [this]
	{
		// 保存の失敗を黙って無視しない（以前は結果を見ていなかった。進み具合が残らないと次回にやり直しになる）
		if ( ! save_data_->save_file( "save/blue-sky.save" ) )
		{
			debug_log( "[Save] セーブデータを保存できませんでした: save/blue-sky.save" );
		}
	};

	hooks.update_cursor = [this] { update_cursor_state(); };

	hooks.on_all_overlays_closed = [this]
	{
		input_->set_blocked( false );
		update_cursor_state();
	};

	hooks.quit = [this] { get_app()->close(); };

	hooks.play_sound = [this] ( const char_t* name )
	{
		if ( auto* s = sound_manager_->get_sound( name ) )
		{
			s->play( false );
		}
	};

	hooks.can_return_to_title = [this]
	{
		return
			get_save_data()->get< int >( "stage.0-0", 0 ) > 0 &&
			get_save_data()->get< int >( "stage.0-1", 0 ) > 0 &&
			get_save_data()->get< int >( "stage.0-2", 0 ) > 0;
	};

	scene_stack_ = std::make_unique< SceneStack >( std::move( hooks ) );
}

/**
 * シーンを準備する
 *
 */
void GameMain::setup_scene()
{
	// 共通のシェーダーとフェーダーを準備する。以前は一部のシーン（debug・enemy_ai_test など）だけが呼んでいて、
	// 呼ばないシーン（title など）から始めると、フェーダーにメッシュが無いまま描画して落ちた。
	// シェーダーの生成は GameMain のシングルトンを使うので、GameMain の構築が終わったここで行う
	graphics_manager_->setup_default_shaders();

	if ( true )
	{
		// set_stage_name( "0-1" );
		// setup_scene( "game_play" );
		string_t start_scene = get_config()->get< string_t >( "debug.start_scene", "debug" );

		// 環境変数でも指定できる（自動操作の確認で設定ファイルを書き換えずに済むように）
		if ( dev_options_.start_scene )
		{
			start_scene = * dev_options_.start_scene;
		}

		// ステージも指定できる（game_play はステージ名が必要）
		if ( dev_options_.start_stage )
		{
			set_stage_name( * dev_options_.start_stage );
		}

		// 打ち間違いなどで未登録なら、ベースシーンが無いまま進まないようタイトルにする
		scene_stack_->start( start_scene );
		// setup_scene( "canvas_test" );

		// 開発用 : 起動時にオーバーレイ（例: options）を開く。UI の確認用
		// ベースシーンを数フレーム更新してから開く（一度も更新されていないシーンは描画の準備ができていないため）。
		// const で読む（非 const の get は既定値の空文字列を設定に書き込み、保存すると壊れた行になるため）
		string_t start_overlay = static_cast< const Config& >( * get_config() ).get< string_t >( "debug.start_overlay", "" );

		// 環境変数でも指定できる（自動操作の確認で設定ファイルを書き換えずに済むように）
		if ( dev_options_.start_overlay )
		{
			start_overlay = * dev_options_.start_overlay;
		}

		scene_stack_->set_start_overlay( start_overlay );

		// 開発用 : BLUE_SKY_DEBUG_SCENE_SEQUENCE（"stage_select,game_play:1-1,title" のように「シーン[:ステージ]」を , で並べる）を設定すると、
		// 150 フレームごとに順にベースシーンを置き換える。シーンを替えて戻ったときにリソースが正しく引けるかを、入力を送らずに確かめるため
		debug_scene_sequence_.assign( dev_options_.scene_sequence.begin(), dev_options_.scene_sequence.end() );

		setup_frame_capture();
		setup_frame_time_measurement();

		return;
	}

	// Scene
	if ( get_save_data()->get< int >( "stage.0-2", 0 ) > 0 )
	{
		scene_stack_->setup_base_scene( "title" );
	}
	else
	{
		if ( get_save_data()->get< int >( "stage.0-1", 0 ) > 0 )
		{
			set_stage_name( "0-2" );
		}
		else if ( get_save_data()->get< int >( "stage.0-0", 0 ) > 0 )
		{
			set_stage_name( "0-1" );
		}
		else
		{
			set_stage_name( "0-0" );
		}

		scene_stack_->setup_base_scene( "stage_intro" );
	}
}

/**
 * ベースを壊した後、次のシーンを作る前の片付け（SceneStack がベースを置き換えるときに呼ぶ）
 */
void GameMain::clear_for_next_scene()
{
	// ゲームの物体を EntityManager より先に壊す（物体の位置の正・剛体の持ち主は、物体の Entity の部品にある。
	// 本編のシーンは自分で壊すが、CanvasTestScene などはスクリプトで作った物体を残したまま終わり、
	// 以前は Entity を失った物体が次のシーンへ残っていた）
	if ( object_factory_ )
	{
		object_factory_->clear();
	}

	entity_manager_->clear();
	get_graphics_manager()->set_main_camera_info( nullptr, nullptr );

	// 物体の剛体は部品が持ち、ECS の剛体は clear で PhysicsSystem が解放する（以前はシーンをまたいで溜まっていた）。
	// 世界が持つ地面・ステージの当たりの形・共有する形はここで消す（以前の本編の世界は GamePlayScene だけが消し、
	// DebugScene・ParticleSystemTestScene が足した地面は残っていた。世界を 1 つにしたので、どのシーンの後も消す）
	const int count = physics_manager_->get_rigid_body_count() - static_cast< int >( physics_manager_->get_owned_rigid_body_count() );

	physics_manager_->clear();

	// 残っていたら漏れなので知らせる
	if ( count > 0 )
	{
		debug_log( "[Physics] シーンを片付けた後も物理の世界に物体の剛体が残っています: " + std::to_string( count ) + " 個" );
	}

	// 前のシーンが解放を頼んだリソースを、次のシーンを読む前に解放する（同時に持つメモリを増やさない）
	flush_resource_releases();

	// Handle の数（中身のある数/作った数）。解放しても減らないので、増え方をここで見る（doc/ハンドルの設計.md の 2 章）
	debug_log( "[Resource] 前のシーンを片付けた後の Handle の数 : " + get_graphics_manager()->describe_handle_counts()
		+ "・音 " + std::to_string( sound_manager_->get_live_handle_count() ) + "/" + std::to_string( sound_manager_->get_handle_count() ) );
}

/**
 * カーソルの表示と閉じ込めを、今の状態（コマンドモード・メニュー・シーンの要求）から決める
 *
 * 以前は 4 か所がそれぞれ決めていて、メニューを表示中にコマンドモードを抜けるとカーソルが消えた。
 */
void GameMain::update_cursor_state()
{
	const bool menu_open = scene_stack_->has_overlay();
	const bool show = is_command_mode_ || menu_open;

	set_show_cursor( show );
	Scene* base = scene_stack_->get_base_scene();
	get_app()->clip_cursor( ! show && base && base->is_clip_cursor_required() );
}

} // namespace blue_sky