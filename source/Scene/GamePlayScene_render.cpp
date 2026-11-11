#include "GamePlayScene.h"

#include <blue_sky/GameMain.h>
#include <game/Config.h>

#include <blue_sky/ecs/EntityObject.h>
#include <blue_sky/ecs/system/PlayerBehaviorSystem.h>
#include <blue_sky/ecs/system/ObjectRenderDataSystem.h>
#include <core/ecs/EntityManager.h>
#include <GameObject/Camera.h>

#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/graphics/Model.h>
#include <blue_sky/graphics/shader/ShadowMapShader.h>

#include <core/graphics/ShadowMap.h>
#include <core/physics/PhysicsManager.h>
#include <blue_sky/DebugLog.h>

#include <core/graphics/Sprite.h>

#include <blue_sky/Input.h>
#include <blue_sky/ObjectFactory.h>

#include <blue_sky/ConstantBuffers.h>

#include <core/graphics/OculusRift.h>

#include <core/math.h>

#include <core/sound/Sound.h>
#include <game/MainLoop.h>

#include <common/math.h>

#include <sstream>

namespace blue_sky
{

/**
 * 描画
 *
 */
void GamePlayScene::render()
{
	stage_timer_.mark( "update と render の間（GameMain）" );

	// 見えない物を描かない（カリング）。カメラは画面に描くときと同じ行列（update_render_data_for_frame）から、影はシャドウマップの今の行列から。
	// Oculus Rift は目ごとに視錐台が違うので、カリングしない（全部を描く）
	if ( ! get_oculus_rift() )
	{
		const Matrix view = Matrix().set_look_at( camera_->position(), camera_->look_at(), camera_->up() );
		const Matrix projection = Matrix().set_perspective_fov( math::degree_to_radian( camera_->fov() ), camera_->aspect(), camera_->near_clip(), camera_->far_clip() );

		get_graphics_manager()->set_object_culling( view * projection );
	}

	get_graphics_manager()->setup_rendering();

	stage_timer_.mark( "描く準備・物体の定数の更新（setup_rendering）" );

	render_to_oculus_vr();
	render_to_display();

	render_text();

	stage_timer_.mark( "文字" );

	if ( stage_timer_.end_frame( get_graphics_manager()->get_draw_count() ) )
	{
		// 測る範囲の最後のフレームの、物理の世界の様子（剛体が眠っているか・広い判定の組の数。BLUE_SKY_DEBUG_FRAME_STAGES のとき）
		const auto stats = get_physics_manager()->get_stats();

		char text[ 256 ];
		sprintf_s( text, "[FrameStage] 物理の世界 : 剛体 %d 個（眠っていない %d・キネマティック %d）・広い判定の組 %d・接触の組 %d（接点あり %d）",
			stats.collision_objects, stats.active_objects, stats.kinematic_objects, stats.overlapping_pairs, stats.manifolds, stats.touching_manifolds );
		debug_log( text );

		// 同じフレームのカリングの結果（描く対象のうち、カメラ・影の各段の範囲に掛かった数）
		if ( const auto* system = get_entity_manager()->get_system< ecs::ObjectRenderDataSystem >() )
		{
			const auto& c = system->get_culling_stats();

			sprintf_s( text, "[FrameStage] カリング : 描く対象 %zu 個（いつも描く %zu）・カメラ %zu・影の段 %zu / %zu / %zu",
				c.targets, c.always_visible, c.in_camera, c.in_shadow_cascade[ 0 ], c.in_shadow_cascade[ 1 ], c.in_shadow_cascade[ 2 ] );
			debug_log( text );
		}
	}
}

/**
 * Oculus Rift に対して描画を行う
 *
 */
void GamePlayScene::render_to_oculus_vr() const
{
	if ( ! get_oculus_rift() )
	{
		return;
	}

	update_render_data_for_frame_drawing();

	get_graphics_manager()->render_shadow_map();

	get_oculus_rift()->setup_rendering();

	// left eye
	get_oculus_rift()->setup_rendering_for_left_eye();
	update_render_data_for_frame_for_eye( 0 );
	render_for_eye();

	// right eye
	get_oculus_rift()->setup_rendering_for_right_eye();
	update_render_data_for_frame_for_eye( 1 );
	render_for_eye();

	get_oculus_rift()->finish_rendering();
}

/**
 * 通常のディスプレイに対して描画を行う
 *
 */
void GamePlayScene::render_to_display() const
{
	update_render_data_for_frame();
	update_render_data_for_frame_drawing();

	stage_timer_.mark( "フレームの定数" );

	get_graphics_manager()->render_shadow_map();

	stage_timer_.mark( "影（シャドウマップ）" );

	get_graphics_manager()->clear_default_view();

	get_graphics_manager()->set_default_render_target();
	get_graphics_manager()->set_default_viewport();
	render_for_eye();
}

/**
 * 各目に対して描画を行う
 *
 * @param ortho_offset UI の両目間オフセット
 */
void GamePlayScene::render_for_eye( float_t ortho_offset ) const
{
	// メッシュと線を分けて呼ぶ（render_objects と同じ。段階ごとの時間を測るため）
	get_graphics_manager()->render_object_meshes();

	stage_timer_.mark( "物体のメッシュ" );

	get_graphics_manager()->render_object_lines();

	stage_timer_.mark( "物体の線" );

	get_graphics_manager()->render_debug_axis();
	get_graphics_manager()->render_debug_bullet();

	get_graphics_manager()->set_input_layout( "main" );

	render_far_billboards();

	get_graphics_manager()->render_background();

	render_sprite( ortho_offset );

	render_fader();

	render_debug_shadow_map_window();

	stage_timer_.mark( "遠景・背景・UI" );
}

void GamePlayScene::render_text() const
{
	std::stringstream ss;
	ss.setf( std::ios_base::fixed, std::ios_base::floatfield );

	if ( get_game_main()->is_display_fps() )
	{
		ss << "FPS : " << get_main_loop()->get_last_fps() << '\n';
		ss << "BPM : " << get_bpm() << '\n';
		const ecs::EntityObject player = get_player_object();

		ss << "POS : " << player.get_location().x() << ", " << player.get_location().y() << ", " << player.get_location().z() << '\n';
		ss << "step speed : " << get_player().step_speed << '\n';
		ss << "last footing height : " << get_player().last_footing_height << '\n';
		ss << "DX : " << player.get_velocity().x() << '\n';
		ss << "DY : " << player.get_velocity().y() << '\n';
		ss << "DZ : " << player.get_velocity().z() << '\n';
		ss << "VELOCITY : " << player.get_velocity().length() << '\n';
		ss << "Objects : " << get_object_factory()->get_entity_object_list().size() << '\n';

		ss << "mouse.dx : " << get_input()->get_mouse_dx() << '\n';
		ss << "mouse.dy : " << get_input()->get_mouse_dy() << '\n';

		ss << "IS LOCATED ON DIE : " << get_player().is_located_on_die << '\n';
		ss << "IS LOCATED ON SAFE : " << get_player().is_located_on_safe << '\n';
		ss << "IS FALLING TO DIE : " << get_player_system()->is_falling_to_die( player_id_ ) << '\n';

		ss << "TESS FACTOR : " << get_graphics_manager()->get_frame_render_data()->data().tess_factor << '\n';

		if ( get_oculus_rift() )
		{
			ss << "YAW : " << get_oculus_rift()->get_yaw() << '\n';
			ss << "PITCH : " << get_oculus_rift()->get_pitch() << '\n';
			ss << "ROLL : " << get_oculus_rift()->get_roll() << '\n';

			ss << "DELTA YAW : " << get_oculus_rift()->get_delta_yaw() << '\n';
		}
	}

	get_graphics_manager()->draw_text( 10.f, 10.f, get_graphics_manager()->get_screen_width() - 10.f, get_graphics_manager()->get_screen_height() - 10.f, ss.str().c_str(), Color( 1.f, 0.95f, 0.95f, 1.f ) );
}

/**
 * フレーム毎に更新する必要のある描画用の定数バッファを更新する
 *


 */
void GamePlayScene::update_render_data_for_frame() const
{
	auto& frame_render_data = get_graphics_manager()->get_frame_render_data()->data();
	update_frame_constant_buffer_data_sub( frame_render_data );

	const Vector& eye = camera_->position();
	const Vector& at  = camera_->look_at();
	const Vector& up  = camera_->up();

	frame_render_data.view = ( Matrix().set_look_at( eye, at, up ) );
	frame_render_data.projection = Matrix().set_perspective_fov( math::degree_to_radian( camera_->fov() ), camera_->aspect(), camera_->near_clip(), camera_->far_clip() );

	get_graphics_manager()->get_frame_render_data()->update();

	get_graphics_manager()->set_eye_position( eye );
}

/**
 * フレーム毎に更新する必要のある描画用の定数バッファを更新する
 *
 * @param int eye_index 0 : 左目 / 1 : 右目
 */
void GamePlayScene::update_render_data_for_frame_for_eye( int eye_index ) const
{
	auto& frame_render_data = get_graphics_manager()->get_frame_render_data()->data();
	update_frame_constant_buffer_data_sub( frame_render_data );

	Matrix camera_rot;
	camera_rot.set_rotation_xyz(
		math::degree_to_radian( camera_->rotate_degree().x() ),
		math::degree_to_radian( camera_->rotate_degree().y() ),
		math::degree_to_radian( camera_->rotate_degree().z() ) );

	Vector eye_offset = ( get_oculus_rift()->get_eye_position( eye_index ) ) * camera_rot;

	Matrix r = get_oculus_rift()->get_eye_rotation( eye_index ) * camera_rot;
	const Vector eye = camera_->position() + eye_offset;
	Vector at = eye + Vector( 0.f, 0.f, 1.f, 0.f ) * r;
	Vector up = Vector( 0.f, 1.f, 0.f, 0.f ) * r;

	frame_render_data.view = Matrix().set_look_at( eye, at, up );
	frame_render_data.projection = get_oculus_rift()->get_projection_matrix( eye_index, camera_->near_clip(), camera_->far_clip() );

	get_graphics_manager()->get_frame_render_data()->update();

	get_graphics_manager()->set_eye_position( eye );
}

/**
 * フレーム毎に更新する必要のある描画用の定数バッファ用データのうち、マトリックス以外のデータをを更新する
 *
 */
void GamePlayScene::update_frame_constant_buffer_data_sub( FrameConstantBufferData& frame_constant_buffer_data ) const
{
	const Vector3 light_position = look_.get_light_position();
	frame_constant_buffer_data.light = -Vector( light_position.x(), light_position.y(), light_position.z(), 1.f );
	frame_constant_buffer_data.light.normalize();
	frame_constant_buffer_data.time = get_total_elapsed_time();
	frame_constant_buffer_data.time_beat = static_cast< uint_t >( get_total_elapsed_time() * ( get_bpm() / 60.f ) );
}
/**
 * フレーム毎に更新する必要のある描画用の定数バッファのうち、手書き風描画に関する定数バッファ更新する
 *
 */
void GamePlayScene::update_render_data_for_frame_drawing() const
{
	const Sound* bgm = resolve( bgm_, "bgm" );
	look_.apply_shadow_and_accent( * get_graphics_manager(), bgm ? bgm->get_current_peak_level() : 0.f );

	get_graphics_manager()->get_frame_drawing_render_data()->update();
}

/**
 * 遠景ビルボードメッシュを描画する
 *
 */
void GamePlayScene::render_far_billboards() const
{
	const Model* far_billboards = resolve( far_billboards_, "far-billboards" );

	if ( ! far_billboards )
	{
		return;
	}

	ObjectConstantBufferData buffer;
	buffer.color = Color::White;
	buffer.world.set_identity();

	get_graphics_manager()->get_shared_object_render_data()->update( & buffer );

	get_graphics_manager()->set_current_object_constant_buffer( get_graphics_manager()->get_shared_object_render_data() );
	far_billboards->render();
}

/**
 * 2D スプライトを描画する
 *
 */
void GamePlayScene::render_sprite( float_t ortho_offset ) const
{
	if ( get_player().action_mode == ecs::PlayerBehaviorComponent::ActionMode::SCOPE )
	{
		get_graphics_manager()->set_input_layout( "main" );

		ObjectConstantBufferData buffer_data;
		buffer_data.world = Matrix().set_orthographic( camera_->aspect() * 2.f, 2.f, 0.f, 1.f );
		buffer_data.world *= Matrix().set_translation( ortho_offset, 0.f, 0.f );

		get_graphics_manager()->get_shared_object_render_data()->update( & buffer_data );

		render_technique( "|main2d", [this]
		{
			bind_shared_object_render_data();

			if ( const Model* scope = resolve( scope_mesh_, "scope" ) )
			{
				scope->render();
			}
		} );
	}

	auto* sprite = get_graphics_manager()->get_sprite();
	sprite->set_ortho_offset( ortho_offset * 20.f );

	sprite->begin();

	// UI の画像はハンドルから引く（引けなければ Sprite は描かない）
	const Texture* ui_texture = resolve( ui_texture_, "ui" );

	render_technique( "|sprite", [this, sprite, ui_texture]
	{
		if ( get_player_inventory().selected_item_type == ecs::PlayerBehaviorComponent::ItemType::ROCKET )
		{
			for ( int n = 0; n < get_player_inventory().get_item_count( ecs::PlayerBehaviorComponent::ItemType::ROCKET ); n++ )
			{
				const int offset = n * 20;

				win::Rect src_rect = win::Rect::Size( 0, 0, 202, 200 );
				win::Point dst_point( get_width() - src_rect.width() - 5, get_height() - src_rect.height() - offset - 5 );

				sprite->draw( dst_point, ui_texture, src_rect.get_rect() );
			}
		}
		else if ( get_player_inventory().selected_item_type == ecs::PlayerBehaviorComponent::ItemType::UMBRELLA )
		{
			for ( int n = 0; n < get_player_inventory().get_item_count( ecs::PlayerBehaviorComponent::ItemType::UMBRELLA ); n++ )
			{
				const float offset = n * 50.f;

				win::Rect src_rect = win::Rect::Size( 0, 256, 186, 220 );

				Matrix t;
				t.set_translation( get_width() - src_rect.width() * 0.5f, get_height() - src_rect.height() * 0.5f - offset, 0.f );

				sprite->set_transform( t );
				sprite->draw( ui_texture, src_rect.get_rect(), Color( 1.f, 1.f, 1.f, 0.75f ) );
			}
		}
		else if ( get_player_inventory().selected_item_type == ecs::PlayerBehaviorComponent::ItemType::STONE )
		{
			for ( int n = get_player_inventory().get_item_count( ecs::PlayerBehaviorComponent::ItemType::STONE ) - 1; n >= 0; --n )
			{
				const int offset = n * 50;

				win::Rect src_rect = win::Rect::Size( 256, 96, 128, 96 );
				win::Point dst_point( get_width() - src_rect.width() - 5, get_height() - src_rect.height() - 5 - offset );

				sprite->draw( dst_point, ui_texture, src_rect.get_rect(), Color( 1.f, 1.f, 1.f, 0.75f ) );
			}
		}
		else if ( get_player_inventory().selected_item_type == ecs::PlayerBehaviorComponent::ItemType::SCOPE )
		{
			win::Rect src_rect = win::Rect::Size( 256, 256, 192, 140 );
			win::Point dst_point( get_width() - src_rect.width() - 5, get_height() - src_rect.height() - 5 );

			sprite->draw( dst_point, ui_texture, src_rect.get_rect(), Color( 1.f, 1.f, 1.f, 0.75f ) );
		}

		if ( get_player_inventory().selected_item_type == ecs::PlayerBehaviorComponent::ItemType::ROCKET || ( get_player_inventory().selected_item_type == ecs::PlayerBehaviorComponent::ItemType::STONE && get_player().can_throw ) )
		{
			// aim
			win::Rect src_rect = win::Rect::Size( 256, 0, 76, 80 );
			win::Point dst_point( ( get_width() - src_rect.width() ) / 2, ( get_height() - src_rect.height() ) / 2 );

			sprite->draw( ui_texture, src_rect.get_rect(), Color( 1.f, 1.f, 1.f, 0.5f ) );
		}

		if ( get_player_inventory().has_medal )
		{
			win::Rect src_rect = win::Rect::Size( 384, 0, 64, 64 );
			win::Point dst_point( 5, get_height() - src_rect.height() - 5 );

			sprite->draw( dst_point, ui_texture, src_rect.get_rect(), Color( 1.f, 1.f, 1.f, 0.75f ) );
		}
	} );

	sprite->end();
}

/**
 * シャドウマップのデバッグウィンドウを描画する
 *
 */
void GamePlayScene::render_debug_shadow_map_window() const
{
	// 開発用の表示なので、設定（blue-sky.config の graphics.debug_shadow_map 1）で有効にしたときだけ出す
	// （以前は影が有効なら常に画面の上に出ていて、プレイヤーにも見えていた）
	if ( ! get_graphics_manager()->is_shadow_enabled() || static_cast< const game::Config& >( * get_config() ).get< int >( "graphics.debug_shadow_map", 0 ) == 0 )
	{
		return;
	}

	get_graphics_manager()->set_viewport( 0.f, 0, get_width() / 4.f * get_graphics_manager()->get_shadow_map()->get_cascade_levels(), get_height() / 4.f );

	// シェーダー・矩形はハンドルから引く（引けなければ描かない）
	auto* debug_texture_shader = get_graphics_manager()->resolve_shader( debug_texture_shader_ );
	const Model* rectangle = resolve( rectangle_, "rectangle" );

	if ( debug_texture_shader && rectangle )
	{
		debug_texture_shader->set_texture_at( 0, get_graphics_manager()->get_shadow_map()->get_texture() );
		debug_texture_shader->render_model( rectangle );
	}

	get_graphics_manager()->set_default_viewport();

}

} // namespace blue_sky
