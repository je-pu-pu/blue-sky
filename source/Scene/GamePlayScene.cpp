#include "GamePlayScene.h"
#include "StageSelectScene.h"

#include <blue_sky/GameMain.h>
#include <blue_sky/GameSettings.h>
#include <core/setting/SettingStore.h>

#include <GameObject/Player.h>
#include <GameObject/Balloon.h>
#include <GameObject/Camera.h>

#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/graphics/Rectangle.h>
#include <blue_sky/graphics/Model.h>

#include <core/graphics/ShadowMap.h>

#include <core/sound/SoundManager.h>

#include <core/graphics/Sprite.h>

#include <blue_sky/ActiveObjectPhysics.h>

#include <blue_sky/Input.h>

#include <blue_sky/ActiveObjectManager.h>
#include <blue_sky/ObjectFactory.h>
#include <blue_sky/DebugLog.h>
#include <GameObject/BaseSwitch.h>
#include <GameObject/Switch.h>
#include <GameObject/Robot.h>

#include <core/graphics/OculusRift.h>

#include <blue_sky/DelayedCommand.h>

#include <core/math.h>

#include <core/sound/Sound.h>
#include <game/Config.h>

#include <common/exception.h>
#include <common/serialize.h>
#include <common/math.h>

namespace blue_sky
{

GamePlayScene::GamePlayScene( GameMain& game_main )
	: Scene( game_main )
	, stage_config_( new Config() )
	, debug_texture_shader_( get_graphics_manager()->get_shader( "debug_shadow_map_texture" ) )
	, camera_( new Camera() )
	, action_bgm_after_timer_( 0.f )
	, bpm_( 120.f )
	, drawing_accent_scale_( 0.f )
	, light_position_( Vector3( 0.25f, 1.f, -1.f ), 0.1f )
	, ambient_color_( Color( 1.f, 1.f, 1.f, 1.f ), 0.02f )
	, shadow_color_( Color( 0.f, 0.f, 0.f, 1.f ), 0.02f )
	, shadow_paper_color_( Color( 0.9f, 0.9f, 0.9f, 1.f ), 0.02f )
	, shading_enabled_( true )
	, balloon_sound_type_( BalloonSoundType::MIX )
	, is_blackout_( false )
	, blackout_timer_( 0.f )
{
	// ShadowMap
	if ( get_config()->get( "graphics.shadow-map-enabled", 1 ) != 0 && stage_config_->get( "graphics.shadow-map-enabled", true ) )
	{
		get_graphics_manager()->setup_shadow_map( get_config()->get( "graphics.shadow-map-cascade-levels", 3 ), get_config()->get( "graphics.shadow-map-size", 1024 ) );
	}

	// Physics
	get_active_object_physics()->add_ground_rigid_body( Vector( 1000, 1, 1000 ) );

	// Texture
	ui_texture_ = get_graphics_manager()->load_named_texture( "ui", "media/image/item.png" );

	// Sound
	load_sound_all( get_stage_name() == "2-3" );


	// Player
	player_ = new Player();
	player_->set_model( get_graphics_manager()->load_model( "player" ) );
	get_active_object_manager()->add_active_object( player_ );
	get_object_factory()->name( "player", player_->get_entity_id() );

	// Goal
	if ( auto id = get_object_factory()->create_object( "goal" ) )
	{
		goal_ = get_object_factory()->get_ref( * id );
		get_object_factory()->name( "goal", * id );
	}
	else
	{
		COMMON_THROW_EXCEPTION_MESSAGE( "create goal failed." );
	}

	// Camera
	camera_->set_fov_default( get_setting_store()->get( game_settings::CAMERA_FOV ) );

	// オプション画面で FOV が変わったら反映する（ズーム倍率は保つ）。購読はシーンと一緒に破棄される
	fov_subscription_ = get_setting_store()->subscribe( game_settings::CAMERA_FOV, [this] ( const core::setting::SettingStore::Value& v )
	{
		camera_->change_fov_default_keep_zoom( std::get< float_t >( v ) );
	} );
	camera_->reset_fov();
	camera_->set_aspect( static_cast< float >( get_width() ) / static_cast< float >( get_height() ) );

	setup_command();

	if ( get_stage_name().empty() )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( "stage name is empty" );
	}

	std::string stage_dir_name = StageSelectScene::get_stage_dir_name_by_page( get_save_data()->get( "stage-select.page", 0 ) );
	load_stage_file( ( stage_dir_name + get_stage_name() + ".stage" ).c_str() );

	if ( ! get_graphics_manager()->is_sky_box_set() )
	{
		get_graphics_manager()->set_sky_box( "sky-box-3" );
	}

	// 既定の遠景。ステージファイルの far_billboards コマンドで差し替える
	far_billboards_ = get_graphics_manager()->load_model( "far-billboards-1" );

	// シャドウマップのデバッグ表示に使う矩形。メッシュは GraphicsManager が作った画面全体の矩形を使い回す
	// （以前は入るたびに同じ名前のモデルとメッシュを作り直していて、古いものが解放されずに溜まり、GraphicsManager の矩形の名前も奪っていた）
	rectangle_ = get_graphics_manager()->get_model( "rectangle" );

	if ( ! rectangle_ )
	{
		rectangle_ = get_graphics_manager()->create_named_model( "rectangle" );
		rectangle_->set_mesh( get_graphics_manager()->get_mesh( "rectangle" ) );
	}

	scope_mesh_ = get_graphics_manager()->load_model( "scope" );

	bgm_ = get_sound_manager()->get_sound( "bgm" );

	if ( bgm_ )
	{
		bgm_->play( stage_config_->get( "bgm.loop", true ) );
	}

	get_graphics_manager()->load_paper_textures();

	restart();
}

GamePlayScene::~GamePlayScene()
{
	get_graphics_manager()->unset_render_target();

	// 物体（ActiveObject と ECS だけの物体）を、物理の世界を片付ける前に壊す
	get_object_factory()->clear();

	get_graphics_manager()->unset_sky_box();
	get_graphics_manager()->unset_ground();
	get_graphics_manager()->unset_shadow_map();

	get_active_object_physics()->clear();

	clear_delayed_command();

	get_graphics_manager()->get_sprite()->set_ortho_offset( 0.f );
}

void GamePlayScene::clear_delayed_command()
{
	delayed_command_list_.clear();
}

/**
 * ステージの設定に応じて、ステージを準備する
 *
 */
void GamePlayScene::setup_stage()
{
	for ( auto i = stage_setup_command_call_list_.begin(); i != stage_setup_command_call_list_.end(); ++i )
	{
		( *i )();
	}
}

void GamePlayScene::restart()
{
	get_graphics_manager()->set_fade_color( Color::White );
	get_graphics_manager()->fade_out();

	is_cleared_ = false;
	action_bgm_after_timer_ = 0.f;

	get_graphics_manager()->set_paper_texture_type( 0 );

	camera_->restart();

	get_object_factory()->restart();

	clear_delayed_command();
	setup_stage();

	play_sound( "restart" );
}

void GamePlayScene::load_sound_all( bool is_final_stage )
{
	if ( is_final_stage )
	{
		get_sound_manager()->load( "medal-get" );

		get_sound_manager()->load( "fin", "girl-see-you" );
	}
	else
	{
		get_sound_manager()->load( "walk" );
		get_sound_manager()->load( "run" );
		get_sound_manager()->load( "clamber" );
		get_sound_manager()->load( "collision-wall" );
		get_sound_manager()->load( "jump" );
		get_sound_manager()->load( "short-breath-jump" );
		get_sound_manager()->load( "land" );
		get_sound_manager()->load( "short-breath" );

		get_sound_manager()->load( "fall" );
		get_sound_manager()->load( "damage-1" );
		get_sound_manager()->load( "dead" );

		get_sound_manager()->load( "balloon" );
		get_sound_manager()->load( "balloon-1" );
		get_sound_manager()->load( "balloon-2" );
		get_sound_manager()->load( "balloon-3" );
		get_sound_manager()->load( "balloon-4" );
		get_sound_manager()->load( "balloon-5" );
		get_sound_manager()->load( "balloon-6" );
		get_sound_manager()->load( "balloon-7" );
		get_sound_manager()->load( "balloon-burst" );

		get_sound_manager()->load( "rocket-get" );
		get_sound_manager()->load( "rocket" );
		get_sound_manager()->load( "rocket-burst" );

		get_sound_manager()->load( "umbrella-get" );
		get_sound_manager()->load( "umbrella-open" );

		get_sound_manager()->load( "stone-get" );
		get_sound_manager()->load( "stone-throw" );

		get_sound_manager()->load( "switch-on" );
		get_sound_manager()->load( "switch-off" );

		get_sound_manager()->load( "ladder-contact" );
		get_sound_manager()->load( "ladder-step" );

		get_sound_manager()->load( "medal-get" );

		get_sound_manager()->load_3d_sound( "soda-can-long-1" );
		get_sound_manager()->load_3d_sound( "soda-can-long-2" );
		get_sound_manager()->load_3d_sound( "soda-can-short-1" );
		get_sound_manager()->load_3d_sound( "soda-can-short-2" );
		get_sound_manager()->load_3d_sound( "soda-can-short-3" );

		get_sound_manager()->load_3d_sound( "robot-chase" );
		get_sound_manager()->load_3d_sound( "robot-found" );
		get_sound_manager()->load_3d_sound( "robot-shutdown" );

		get_sound_manager()->load( "fin" );
		get_sound_manager()->load( "door" );
	}
}

/**
 * メインループ処理
 *
 */
void GamePlayScene::update()
{
	Scene::update();

	if ( is_blackout_ )
	{
		update_blackout();
	}
	else if ( is_cleared_ )
	{
		update_clear();
	}
	else
	{
		update_main();
	}

	// 開発用 : BLUE_SKY_DEBUG_PRESS_SWITCHES を設定すると、始まってから 30 フレーム後にステージのスイッチを全部押す
	// （スイッチで壁やゴールが動くイベントを、遊ばずに確かめるため。動いた物体が目標に着くと log/blue-sky.log に出る）
	if ( debug_press_switches_frame_ >= 0 && GetEnvironmentVariableA( "BLUE_SKY_DEBUG_PRESS_SWITCHES", nullptr, 0 ) > 0 && ++debug_press_switches_frame_ == 30 )
	{
		debug_press_switches_frame_ = -1;

		// 加えた順（ステージに書いた順）に押す。スイッチのイベントで物体が作られる（一覧に加わる）ことがあるので、添字で回す
		const auto& objects = get_active_object_manager()->active_object_list();

		for ( size_t i = 0; i < objects.size(); ++i )
		{
			if ( auto* s = dynamic_cast< BaseSwitch* >( objects[ i ] ) )
			{
				const bool switched = s->do_switch();
				debug_log( string_t( "[Debug] スイッチを押しました : " ) + ( switched ? "入った" : "入らなかった" ) );
			}
		}
	}

	debug_visit_objects();
	debug_throw_stones();

	get_active_object_manager()->update();

	if ( ! is_cleared_ )
	{
		get_graphics_manager()->clear_debug_bullet();
		get_active_object_physics()->update( get_elapsed_time() );

		// 剛体の位置を位置の正（TransformComponent）へ写す（ecs::PhysicsSyncSystem。以前は物体ごとに update_transform を呼んでいた）
		get_active_object_manager()->update_transforms();
	}

	camera_->update_with_player( player_ );
	camera_->update();


	// collision_check（当たった組を ecs::CollisionEventSystem に集め、互換の層が二重ディスパッチを呼ぶ。調べる順は以前と同じ : プレイヤー → ゴール → 全部）
	get_active_object_manager()->check_collision_with( player_ );

	if ( get_active_object_physics()->is_collision( player_->get_rigid_body(), goal_.get_rigid_body() ) )
	{
		on_goal();
	}

	get_active_object_manager()->check_collision_all();

	get_sound_manager()->set_listener_position( camera_->position().xyz() );
	get_sound_manager()->set_listener_orientation( camera_->front().xyz(), camera_->up().xyz() );
	get_sound_manager()->commit();

	update_delayed_command();
	update_balloon_sound();
	update_shadow();

	light_position_.chase();
	ambient_color_.chase();
	shadow_color_.chase();
	shadow_paper_color_.chase();

	get_graphics_manager()->set_ambient_color( ambient_color_.value() );

	// tess test
	{
		if ( get_input()->press( Input::Button::L2 ) )
		{
			get_graphics_manager()->get_frame_render_data()->data().tess_factor -= 1 * get_elapsed_time();
		}
		if ( get_input()->press( Input::Button::R2 ) )
		{
			get_graphics_manager()->get_frame_render_data()->data().tess_factor += 1 * get_elapsed_time();
		}

		get_graphics_manager()->get_frame_render_data()->data().tess_factor = math::clamp( get_graphics_manager()->get_frame_render_data()->data().tess_factor, 1.f, 8.f );
	}
}

/**
 * 通常時の更新処理
 *
 */
void GamePlayScene::update_main()
{
	const float_t rotation_speed_rate = camera_->fov() / camera_->get_fov_default();

	if ( ! get_oculus_rift() )
	{
		camera_->rotate_degree_target() += Vector( get_input()->get_mouse_dy() * get_mouse_rotation_speed() * rotation_speed_rate, 0.f, 0.f, 0.f );
		camera_->rotate_degree_target().set_x( math::clamp( camera_->rotate_degree_target().x(), -get_mouse_rotation_speed(), +get_mouse_rotation_speed() ) );
	}

	player_->set_pitch( -camera_->rotate_degree_target().x() / get_mouse_rotation_speed() );

	float eye_depth_add = 0.f;

	if ( player_->is_on_ladder() )
	{
		eye_depth_add = -0.05f;
	}
	else if ( camera_->rotate_degree_target().x() > 0.f )
	{
		if ( ! get_oculus_rift() )
		{
			eye_depth_add = get_input()->get_mouse_dy();
		}
	}

	player_->add_eye_depth( eye_depth_add );

	if ( ! player_->is_dead() )
	{
		bool is_moving = false;

		if ( ! player_->is_ladder_step_only() )
		{
			if ( get_input()->press( Input::Button::LEFT ) )
			{
				player_->side_step( -1.f );
				is_moving = true;
			}
			if ( get_input()->press( Input::Button::RIGHT ) )
			{
				player_->side_step( +1.f );
				is_moving = true;
			}
			if ( get_input()->press( Input::Button::UP ) )
			{
				player_->step( +1.f );
				is_moving = true;
			}
			if ( get_input()->press( Input::Button::DOWN ) )
			{
				player_->step( -1.f );
				is_moving = true;
			}
		}

		bool is_moving_on_ladder = false;

		if ( player_->is_on_ladder() )
		{
			if ( get_input()->press( Input::Button::UP ) )
			{
				player_->ladder_step( +1.f );
				is_moving_on_ladder = true;
			}
			if ( get_input()->press( Input::Button::DOWN ) )
			{
				player_->ladder_step( -1.f );
				is_moving_on_ladder = true;
			}
		}

		if ( ! is_moving )
		{
			player_->stop();
		}

		if ( ! is_moving_on_ladder )
		{
			player_->stop_ladder_step();
		}

		if ( get_input()->push( Input::Button::A ) )
		{
			switch ( player_->get_selected_item_type() )
			{
				case Player::ItemType::NONE: player_->jump(); break;
				case Player::ItemType::ROCKET: player_->rocket( camera_->front() ); break;
				case Player::ItemType::STONE: player_->throw_stone( camera_->front() ); break;
					case Player::ItemType::SCOPE:
				{
					player_->switch_scope_mode();

					if ( player_->get_action_mode() == Player::ActionMode::SCOPE )
					{
						camera_->set_fov_target( camera_->get_fov_default() * get_scope_zoom_factor() );
						camera_->set_fov( camera_->get_fov_default() * get_scope_zoom_factor() );
					}
					break;
				}
			}
		}
		else if ( get_input()->push( Input::Button::JUMP ) )
		{
			player_->jump();
		}
		else if ( get_input()->press( Input::Button::A ) || get_input()->press( Input::Button::JUMP ) )
		{
			player_->clamber();
		}
		else
		{
			player_->stop_clamber();
		}

		int wheel = get_input()->pop_mouse_wheel_queue();

		if ( player_->get_action_mode() == Player::ActionMode::SCOPE )
		{
			if ( wheel > 0 )
			{
				camera_->set_fov_target( std::max( camera_->get_fov_target() * get_scope_zoom_factor(), get_min_scope_fov() ) );
			}
			else if ( wheel < 0 )
			{
				camera_->set_fov_target( std::min( camera_->get_fov_target() / get_scope_zoom_factor(), camera_->get_fov_default() * get_scope_zoom_factor() ) );
			}
		}
		else
		{
			if ( wheel > 0 )
			{
				player_->select_next_item();
			}
			else if ( wheel < 0 )
			{
				player_->select_prev_item();
			}

			camera_->reset_fov();
		}

		float_t add_direction_degree_by_mouse = get_input()->get_mouse_dx() * get_mouse_rotation_speed() * rotation_speed_rate;
		float_t add_direction_degree_by_hmd   = get_oculus_rift() ? math::radian_to_degree( get_oculus_rift()->get_delta_yaw() ) : 0.f;

		player_->add_direction_degree( add_direction_degree_by_mouse + add_direction_degree_by_hmd );
		camera_->rotate_degree_target().set_y( player_->get_direction_degree() );

		if ( player_->is_falling_to_die() )
		{
			// 落ちて死のうとしている場合は白くフェードアウトする

			get_graphics_manager()->set_fade_color( Color( 1.f, 1.f, 1.f, 0.5f ) );
			get_graphics_manager()->fade_out( get_fade_speed_falling() );
		}
		else
		{
			get_graphics_manager()->fade_in( get_fade_speed_normal() );
		}
	}
	else
	{
		if ( get_input()->push( Input::Button::A ) )
		{
			restart();
		}
		else
		{
			get_graphics_manager()->set_fade_color( Color( 0.25f, 0.f, 0.f, 0.75f ) );
			get_graphics_manager()->fade_out( get_fade_speed_dead() );
		}
	}
}

void GamePlayScene::update_blackout()
{
	blackout_timer_ += get_elapsed_time();

	get_graphics_manager()->set_fade_color( Color::Black );
	get_graphics_manager()->fade_out();

	if ( blackout_timer_ >= get_blackout_timeout() )
	{
		go_to_next_scene();
	}
}

void GamePlayScene::update_delayed_command()
{
	for ( auto i = delayed_command_list_.begin(); i != delayed_command_list_.end(); )
	{
		if ( ( *i )->update( get_elapsed_time() ) )
		{
			exec_command( ( *i )->get_command() );
		}

		if ( ( *i )->is_over() )
		{
			i = delayed_command_list_.erase( i );
		}
		else
		{
			++i;
		}
	}
}

void GamePlayScene::update_balloon_sound()
{
	int_t balloon_sound_request = player_->pop_balloon_sound_request();

	if ( balloon_sound_request >= 1 )
	{
		if ( balloon_sound_type_ == BalloonSoundType::MIX || balloon_sound_type_ == BalloonSoundType::SOLO )
		{
			Sound* sound = get_sound_manager()->get_sound( "balloon" );

			if ( sound )
			{
				sound->play( false );
			}
		}
		else if ( balloon_sound_type_ == BalloonSoundType::SCALE )
		{
			int r = math::clamp( balloon_sound_request, 1, 7 );

			for ( int n = 1; n <= 7; n++ )
			{
				if ( n != r )
				{
					stop_sound( ( std::string( "balloon-" ) + common::serialize( n ) ).c_str() );
				}
			}

			play_sound( ( std::string( "balloon-" ) + common::serialize( r ) ).c_str() );
		}
	}

	if ( player_->get_action_mode() == Player::ActionMode::BALLOON && ( balloon_sound_type_ == BalloonSoundType::SOLO || balloon_sound_type_ == BalloonSoundType::SCALE ) )
	{
		action_bgm_after_timer_ = get_action_bgm_fade_delay();
	}
	else
	{
		action_bgm_after_timer_ -= get_elapsed_time();
	}

	if ( action_bgm_after_timer_ > 0.f )
	{
		if ( bgm_ )
		{
			bgm_->fade_out();
		}
	}
	else
	{
		if ( bgm_ )
		{
			bgm_->fade_in();
		}
	}

	// プレイヤーの位置が決まった後で、持っている風船を手元に合わせ直す
	// （物体の更新の中だけだと、プレイヤーより先に更新されたときに 1 フレーム遅れてぶれる。以前は const_cast で呼んでいた）
	if ( auto* balloon = player_->get_balloon() )
	{
		balloon->update();
	}
}

/**
 * 影関連の情報を更新する
 *
 */
void GamePlayScene::update_shadow()
{
	if ( get_graphics_manager()->is_shadow_enabled() )
	{
		get_graphics_manager()->get_shadow_map()->set_light_position( Vector( light_position_.value().x(), light_position_.value().y(), light_position_.value().z(), 1.f ) );
		get_graphics_manager()->get_shadow_map()->set_eye_position( camera_->position() );
	}
}

void GamePlayScene::on_goal()
{
	if ( is_cleared_ )
	{
		return;
	}

	is_cleared_ = true;

	get_sound_manager()->stop_all();

	if ( Sound* fin = get_sound_manager()->get_sound( "fin" ) )
	{
		fin->play( false );
	}
}

/**
 * 開発用 : プレイヤーをメダル・風船・梯子・ゴールの所へ順に動かし、当たった結果をログに出す
 *
 * 環境変数 BLUE_SKY_DEBUG_VISIT_OBJECTS を設定したときだけ動く（本編の動きは変えない）。行き先はメダル・風船・梯子・ロケット・ゴールの順（あるものだけ）。
 * 当たりのイベント（ecs::CollisionEventSystem と互換の層）が、遊ばずに起きることを確かめるため。
 * 始まってから 60 フレーム後から 40 フレームごとに、次の物体の位置へプレイヤーを置いて速度を 0 にする。
 * 置いてから 3 フレーム後に、その結果（メダルを持っているか・風船につかまっているか・梯子に触れているか・ゴールしたか）を出す。
 * ゴールに着くとステージが終わるので、ゴールは最後にする
 */
void GamePlayScene::debug_visit_objects()
{
	if ( debug_visit_objects_frame_ < 0 || GetEnvironmentVariableA( "BLUE_SKY_DEBUG_VISIT_OBJECTS", nullptr, 0 ) == 0 )
	{
		return;
	}

	constexpr int start_frame = 60;
	constexpr int interval = 40;
	constexpr int result_frame = 3;		///< 置いてから結果を出すまでのフレーム数（当たりは物理を進めた後に調べるので、置いた次のフレームから効く）

	const int frame = debug_visit_objects_frame_++;
	const int phase = ( frame - start_frame ) % interval;

	if ( frame < start_frame || ( phase != 0 && phase != result_frame ) )
	{
		return;
	}

	// 行き先（種類ごとに最初の 1 つ。加えた順 = ステージに書いた順）
	std::vector< std::pair< string_t, Vector > > targets;

	auto add_first = [ & ] ( const char_t* name, auto is_kind )
	{
		for ( const auto* o : get_active_object_manager()->active_object_list() )
		{
			if ( is_kind( o ) )
			{
				targets.emplace_back( name, o->get_location() );
				return;
			}
		}
	};

	// ECS だけの物体になった種類は、工場が作った物体から種類の名前で探す
	auto add_first_entity = [ & ] ( const char_t* name, const char_t* kind )
	{
		if ( auto id = get_object_factory()->find_first_of_kind( kind ) )
		{
			targets.emplace_back( name, get_object_factory()->get_ref( * id ).get_location() );
		}
	};

	add_first_entity( "メダル", "medal" );
	add_first( "風船", [] ( const ActiveObject* o ) { return dynamic_cast< const Balloon* >( o ) != nullptr; } );
	add_first_entity( "梯子", "ladder" );
	add_first_entity( "ロケット", "rocket" );
	targets.emplace_back( "ゴール", goal_.get_location() );

	const size_t step = static_cast< size_t >( ( frame - start_frame ) / interval );

	if ( phase == result_frame )
	{
		if ( step < targets.size() )
		{
			debug_log( string_t( "[Debug] 物体の所へ動かした結果 : " ) + targets[ step ].first
				+ " / メダル " + ( player_->has_medal() ? "持っている" : "持っていない" )
				+ " / 風船 " + ( player_->get_action_mode() == Player::ActionMode::BALLOON ? "つかまっている" : "つかまっていない" )
				+ " / 梯子 " + ( player_->is_on_ladder() ? "触れている" : "触れていない" )
				+ " / ゴール " + ( is_cleared_ ? "した" : "していない" )
				+ " / ロケット " + std::to_string( player_->get_item_count( Player::ItemType::ROCKET ) ) + " 個" );
		}

		return;
	}

	if ( step >= targets.size() || is_cleared_ )
	{
		debug_visit_objects_frame_ = -1;
		return;
	}

	const Vector to = targets[ step ].second;

	player_->set_location( to );
	player_->set_velocity( Vector::Zero );

	debug_log( "[Debug] プレイヤーを" + targets[ step ].first + "の所へ動かしました ( " + std::to_string( to.x() ) + ", " + std::to_string( to.y() ) + ", " + std::to_string( to.z() ) + " )" );
}

/**
 * 開発用 : ステージの石を全部拾い、スイッチ・敵・風船に向けて順に投げて、当たった結果をログに出す
 *
 * 環境変数 BLUE_SKY_DEBUG_THROW_STONES を設定したときだけ動く（本編の動きは変えない）。
 * 石を拾う・投げる・石がスイッチや敵や風船に当たる、を遊ばずに確かめるため（石とスイッチを ECS だけの物体にするときの見比べ）。
 * 始まってから 60 フレーム後から 10 フレームごとに、次の石の位置へプレイヤーを置く（触れると拾う）。
 * 拾い終えたら、行き先（最初のスイッチ・全部の敵・最初の風船。あるものだけ）ごとに 60 フレームずつ使う :
 * 0 フレーム目に行き先の 2.5 m 後ろ（向きを持たない物は -z の側）へ置いて行き先を向かせ、5 フレーム目に行き先へ投げ、45 フレーム目に結果を出す
 */
void GamePlayScene::debug_throw_stones()
{
	if ( debug_throw_stones_frame_ < 0 || GetEnvironmentVariableA( "BLUE_SKY_DEBUG_THROW_STONES", nullptr, 0 ) == 0 )
	{
		return;
	}

	constexpr int start_frame = 60;
	constexpr int pick_interval = 10;
	constexpr int throw_interval = 60;
	constexpr int throw_frame = 5;
	constexpr int result_frame = 45;
	constexpr float_t distance = 2.5f;

	const int frame = debug_throw_stones_frame_++;

	if ( frame < start_frame )
	{
		return;
	}

	// 種類の物体を作った順に集める（まだ ActiveObject の種類は型で、ECS だけの物体は種類の名前で探す）
	auto find_objects = [ & ] ( const char_t* kind, auto is_kind )
	{
		std::vector< GameObjectRef > result;

		for ( const auto* o : get_active_object_manager()->active_object_list() )
		{
			if ( is_kind( o ) )
			{
				result.push_back( get_object_factory()->get_ref( o->get_entity_id() ) );
			}
		}

		for ( const auto id : get_object_factory()->find_all_of_kind( kind ) )
		{
			result.push_back( get_object_factory()->get_ref( id ) );
		}

		return result;
	};

	const auto stones = find_objects( "stone", [] ( const ActiveObject* ) { return false; } );

	// 石を拾う
	if ( frame < start_frame + pick_interval * static_cast< int >( stones.size() ) )
	{
		if ( ( frame - start_frame ) % pick_interval == 0 )
		{
			player_->set_location( stones[ ( frame - start_frame ) / pick_interval ].get_location() );
			player_->set_velocity( Vector::Zero );
		}

		return;
	}

	const int throw_start_frame = start_frame + pick_interval * static_cast< int >( stones.size() );

	if ( frame == throw_start_frame )
	{
		debug_log( "[Debug] 石を拾いました : " + std::to_string( player_->get_item_count( Player::ItemType::STONE ) ) + " 個（ステージの石 " + std::to_string( stones.size() ) + " 個）" );
	}

	// 行き先（種類ごとに最初の 1 つ）
	std::vector< std::pair< string_t, GameObjectRef > > targets;

	auto add_first = [ & ] ( const char_t* name, const std::vector< GameObjectRef >& list )
	{
		if ( ! list.empty() )
		{
			targets.emplace_back( name, list.front() );
		}
	};

	add_first( "スイッチ", find_objects( "switch", [] ( const ActiveObject* o ) { return dynamic_cast< const Switch* >( o ) != nullptr; } ) );
	// 敵は全部（向きや周りの壁で投げられない・見つかる敵がいるので）
	for ( const auto& robot : find_objects( "robot", [] ( const ActiveObject* o ) { return dynamic_cast< const Robot* >( o ) != nullptr; } ) )
	{
		targets.emplace_back( "敵 " + std::to_string( targets.size() ), robot );
	}

	add_first( "風船", find_objects( "balloon", [] ( const ActiveObject* o ) { return dynamic_cast< const Balloon* >( o ) != nullptr; } ) );

	const size_t step = static_cast< size_t >( ( frame - throw_start_frame ) / throw_interval );
	const int phase = ( frame - throw_start_frame ) % throw_interval;

	if ( step >= targets.size() )
	{
		debug_log( "[Debug] 石を投げ終えました" );
		debug_throw_stones_frame_ = -1;
		return;
	}

	const string_t& name = targets[ step ].first;
	const GameObjectRef& target = targets[ step ].second;

	if ( phase == 0 )
	{
		// 行き先の後ろ（敵に見つからないように。向きを持たない物は -z の側）に置き、行き先を向かせる
		Vector back( 0.f, 0.f, -1.f );

		if ( const ActiveObject* o = target.get_active_object() )
		{
			back = -o->get_front();
		}

		const Vector from = target.get_location() + back * distance;
		const Vector to = target.get_location() - from;

		player_->set_location( from );
		player_->set_velocity( Vector::Zero );
		player_->set_direction_degree( math::radian_to_degree( std::atan2( to.x(), to.z() ) ) );
	}
	else if ( phase == throw_frame )
	{
		Vector direction = ( target.get_location() + Vector( 0.f, 0.5f, 0.f ) ) - ( player_->get_location() + Vector( 0.f, player_->get_eye_height(), 0.f ) );
		direction.normalize();

		const int count = player_->get_item_count( Player::ItemType::STONE );
		player_->throw_stone( direction );

		debug_log( "[Debug] 石を" + name + "へ" + ( player_->get_item_count( Player::ItemType::STONE ) < count ? "投げました" : "投げられませんでした" ) + "（残り " + std::to_string( player_->get_item_count( Player::ItemType::STONE ) ) + " 個）" );
	}
	else if ( phase == result_frame )
	{
		string_t result = "不明";

		if ( const auto* s = dynamic_cast< const BaseSwitch* >( target.get_active_object() ) )
		{
			result = s->get_state() == BaseSwitch::State::ON ? "入" : s->get_state() == BaseSwitch::State::OFF ? "切" : "壊れた";
		}
		else if ( const auto* r = dynamic_cast< const Robot* >( target.get_active_object() ) )
		{
			result = r->get_mode() == Robot::Mode::SHUTDOWN ? "止まった" : "止まっていない";
		}
		else if ( const auto* b = dynamic_cast< const Balloon* >( target.get_active_object() ) )
		{
			result = b->is_dead() ? "割れた" : "割れていない";
		}

		debug_log( "[Debug] 石を投げた結果 : " + name + " / " + result + "（持っている石 " + std::to_string( player_->get_item_count( Player::ItemType::STONE ) ) + " 個）" );
	}
}

void GamePlayScene::update_clear()
{
	Vector target_position = goal_.get_location();
	target_position.set_z( target_position.z() - get_goal_camera_z_offset() + get_sound_manager()->get_sound( "fin" )->get_current_position() * get_goal_camera_z_speed() );

	player_->set_location( player_->get_location() * ( 1.f - get_goal_position_lerp() ) + target_position * get_goal_position_lerp() );
	player_->set_direction_degree( 0.f );

	camera_->rotate_degree_target().set( 0.f, player_->get_direction_degree(), 0.f, 0.f );
	camera_->set_rotate_chase_speed( get_camera_chase_speed_on_clear() );

	get_graphics_manager()->fade_out( get_fade_speed_clear() );

	Sound* fin_sound = get_sound_manager()->get_sound( "fin" );

	if ( fin_sound )
	{
		if (
			fin_sound->get_current_position() >= get_door_sound_start() &&
			fin_sound->get_current_position() <= get_door_sound_end() &&
			get_sound_manager()->get_sound( "door" ) && ! get_sound_manager()->get_sound( "door" )->is_playing() )
		{
			get_sound_manager()->get_sound( "door" )->play( false );
		}

		if ( ! fin_sound->is_playing() )
		{
			go_to_next_scene();
		}
	}
}

void GamePlayScene::go_to_next_scene()
{
	std::string save_param_name = StageSelectScene::get_stage_prefix_by_page( get_save_data()->get( "stage-select.page", 0 ) ) + "." + get_stage_name();

	get_save_data()->set( save_param_name.c_str(), std::max( player_->has_medal() ? 2 : 1, get_save_data()->get( save_param_name.c_str(), 0 ) ) );

	if ( get_stage_name() == "2-3" )
	{
		set_next_scene( "ending" );
	}
	else
	{
		set_next_scene( "stage_outro" );
	}
}

} // namespace blue_sky
