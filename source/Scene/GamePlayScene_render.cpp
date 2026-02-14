#include "GamePlayScene.h"

#include <blue_sky/GameMain.h>

#include <GameObject/Player.h>
#include <GameObject/Camera.h>

#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/graphics/Model.h>
#include <blue_sky/graphics/shader/ShadowMapShader.h>

#include <core/graphics/ShadowMap.h>

#include <core/graphics/Direct3D11/Sprite.h>

#include <blue_sky/Input.h>
#include <blue_sky/ActiveObjectManager.h>

#include <blue_sky/ConstantBuffers.h>

#include <core/graphics/OculusRift.h>

#include <core/math.h>

#include <game/Sound.h>
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
	get_graphics_manager()->setup_rendering();

	render_to_oculus_vr();
	render_to_display();

	render_text();
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

	get_graphics_manager()->render_shadow_map();

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
	get_graphics_manager()->render_active_objects( get_active_object_manager() );

	get_graphics_manager()->render_debug_axis( get_active_object_manager() );
	get_graphics_manager()->render_debug_bullet();

	get_graphics_manager()->set_input_layout( "main" );

	render_far_billboards();

	get_graphics_manager()->render_background();

	render_sprite( ortho_offset );

	render_fader();

	render_debug_shadow_map_window();
}

void GamePlayScene::render_text() const
{
	std::stringstream ss;
	ss.setf( std::ios_base::fixed, std::ios_base::floatfield );

	if ( get_game_main()->is_display_fps() )
	{
		ss << "FPS : " << get_main_loop()->get_last_fps() << '\n';
		ss << "BPM : " << get_bpm() << '\n';
		ss << "POS : " << player_->get_transform().get_position().x() << ", " << player_->get_transform().get_position().y() << ", " << player_->get_transform().get_position().z() << '\n';
		ss << "step speed : " << player_->get_step_speed() << '\n';
		ss << "last footing height : " << player_->get_last_footing_height() << '\n';
		ss << "DX : " << player_->get_velocity().x() << '\n';
		ss << "DY : " << player_->get_velocity().y() << '\n';
		ss << "DZ : " << player_->get_velocity().z() << '\n';
		ss << "VELOCITY : " << player_->get_velocity().length() << '\n';
		ss << "Objects : " << get_active_object_manager()->active_object_list().size() << '\n';

		ss << "mouse.dx : " << get_input()->get_mouse_dx() << '\n';
		ss << "mouse.dy : " << get_input()->get_mouse_dy() << '\n';

		ss << "IS LOCATED ON DIE : " << player_->is_located_on_die() << '\n';
		ss << "IS LOCATED ON SAFE : " << player_->is_located_on_safe() << '\n';
		ss << "IS FALLING TO DIE : " << player_->is_falling_to_die() << '\n';

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
	frame_constant_buffer_data.light = -Vector( light_position_.value().x(), light_position_.value().y(), light_position_.value().z(), 1.f );
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
	get_graphics_manager()->set_shadow_color( shadow_color_.value() );
	get_graphics_manager()->set_shadow_paper_color( shadow_paper_color_.value() );
	get_graphics_manager()->set_drawing_accent( bgm_ ? bgm_->get_current_peak_level() * drawing_accent_scale_ : 0.f );

	get_graphics_manager()->get_frame_drawing_render_data()->update();
}

/**
 * 遠景ビルボードメッシュを描画する
 *
 */
void GamePlayScene::render_far_billboards() const
{
	if ( ! far_billboards_ )
	{
		return;
	}

	ObjectConstantBufferData buffer;
	buffer.color = Color::White;
	buffer.world.set_identity();

	get_graphics_manager()->get_shared_object_render_data()->update( & buffer );

	get_graphics_manager()->set_current_object_constant_buffer( get_graphics_manager()->get_shared_object_render_data() );
	far_billboards_->render();
}

/**
 * 2D スプライトを描画する
 *
 */
void GamePlayScene::render_sprite( float_t ortho_offset ) const
{
	if ( player_->get_action_mode() == Player::ActionMode::SCOPE )
	{
		get_graphics_manager()->set_input_layout( "main" );

		ObjectConstantBufferData buffer_data;
		buffer_data.world = Matrix().set_orthographic( camera_->aspect() * 2.f, 2.f, 0.f, 1.f );
		buffer_data.world *= Matrix().set_translation( ortho_offset, 0.f, 0.f );

		get_graphics_manager()->get_shared_object_render_data()->update( & buffer_data );

		render_technique( "|main2d", [this]
		{
			bind_shared_object_render_data();

			scope_mesh_->render();
		} );
	}

	auto* sprite = get_graphics_manager()->get_sprite();
	sprite->set_ortho_offset( ortho_offset * 20.f );

	sprite->begin();

	render_technique( "|sprite", [this, sprite]
	{
		if ( player_->get_selected_item_type() == Player::ItemType::ROCKET )
		{
			for ( int n = 0; n < player_->get_item_count( Player::ItemType::ROCKET ); n++ )
			{
				const int offset = n * 20;

				win::Rect src_rect = win::Rect::Size( 0, 0, 202, 200 );
				win::Point dst_point( get_width() - src_rect.width() - 5, get_height() - src_rect.height() - offset - 5 );

				sprite->draw( dst_point, ui_texture_, src_rect.get_rect() );
			}
		}
		else if ( player_->get_selected_item_type() == Player::ItemType::UMBRELLA )
		{
			for ( int n = 0; n < player_->get_item_count( Player::ItemType::UMBRELLA ); n++ )
			{
				const float offset = n * 50.f;

				win::Rect src_rect = win::Rect::Size( 0, 256, 186, 220 );

				Matrix t;
				t.set_translation( get_width() - src_rect.width() * 0.5f, get_height() - src_rect.height() * 0.5f - offset, 0.f );

				sprite->set_transform( t );
				sprite->draw( ui_texture_, src_rect.get_rect(), Color( 1.f, 1.f, 1.f, 0.75f ) );
			}
		}
		else if ( player_->get_selected_item_type() == Player::ItemType::STONE )
		{
			for ( int n = player_->get_item_count( Player::ItemType::STONE ) - 1; n >= 0; --n )
			{
				const int offset = n * 50;

				win::Rect src_rect = win::Rect::Size( 256, 96, 128, 96 );
				win::Point dst_point( get_width() - src_rect.width() - 5, get_height() - src_rect.height() - 5 - offset );

				sprite->draw( dst_point, ui_texture_, src_rect.get_rect(), Color( 1.f, 1.f, 1.f, 0.75f ) );
			}
		}
		else if ( player_->get_selected_item_type() == Player::ItemType::SCOPE )
		{
			win::Rect src_rect = win::Rect::Size( 256, 256, 192, 140 );
			win::Point dst_point( get_width() - src_rect.width() - 5, get_height() - src_rect.height() - 5 );

			sprite->draw( dst_point, ui_texture_, src_rect.get_rect(), Color( 1.f, 1.f, 1.f, 0.75f ) );
		}

		if ( player_->get_selected_item_type() == Player::ItemType::ROCKET || ( player_->get_selected_item_type() == Player::ItemType::STONE && player_->can_throw() ) )
		{
			// aim
			win::Rect src_rect = win::Rect::Size( 256, 0, 76, 80 );
			win::Point dst_point( ( get_width() - src_rect.width() ) / 2, ( get_height() - src_rect.height() ) / 2 );

			sprite->draw( ui_texture_, src_rect.get_rect(), Color( 1.f, 1.f, 1.f, 0.5f ) );
		}

		if ( player_->has_medal() )
		{
			win::Rect src_rect = win::Rect::Size( 384, 0, 64, 64 );
			win::Point dst_point( 5, get_height() - src_rect.height() - 5 );

			sprite->draw( dst_point, ui_texture_, src_rect.get_rect(), Color( 1.f, 1.f, 1.f, 0.75f ) );
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
	if ( ! get_graphics_manager()->is_shadow_enabled() )
	{
		return;
	}

	get_graphics_manager()->set_viewport( 0.f, 0, get_width() / 4.f * get_graphics_manager()->get_shadow_map()->get_cascade_levels(), get_height() / 4.f );

	debug_texture_shader_->set_texture_at( 0, get_graphics_manager()->get_shadow_map()->get_texture() );
	debug_texture_shader_->render_model( rectangle_ );

	get_graphics_manager()->set_default_viewport();

}

} // namespace blue_sky
