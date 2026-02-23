#include "RenderSystem.h"
#include <core/ConstantBuffers.h>
#include <core/Service.h>
#include <core/TimeManager.h>
#include <core/graphics/GraphicsManager.h>
#include <core/graphics/Model.h>
#include <core/graphics/Shader.h>
#include <core/graphics/RenderTargetTexture.h>

#include <common/math.h>

#include <unordered_map>

namespace core::ecs
{

RenderSystem::RenderSystem()
	// : render_result_texture_1_( get_graphics_manager()->create_render_target_texture( core::graphics::PixelFormat::R8_UINT ) )
	: render_result_texture_1_( get_graphics_manager()->create_render_target_texture() )
	, render_result_texture_2_( get_graphics_manager()->create_render_target_texture() )
	, instance_buffer_( std::make_unique< core::graphics::direct_3d_11::InstanceBuffer >() )
{
	//
}

void RenderSystem::update()
{
	auto noise_shader = get_graphics_manager()->get_shader( "post_effect_noise" );
	noise_shader->set_float( "offset", core::get_time_manager()->get_total_elapsed_time() );

	get_graphics_manager()->setup_rendering();

	const auto is_post_effect_enabled = true;

	if ( is_post_effect_enabled )
	{
		render_result_texture_1_->clear();
		get_graphics_manager()->set_render_target( render_result_texture_1_.get() );
	}

	get_graphics_manager()->render_background();

	// カメラ位置を取得してカリングに使用
	Vector camera_position = get_graphics_manager()->get_camera_position();
	Vector camera_forward = get_graphics_manager()->get_camera_forward();

	const float culling_distance_sq = culling_distance_ * culling_distance_;

	// モデルごとにインスタンスをグループ化
	using ModelType = core::graphics::Model;
	std::unordered_map< ModelType*, std::vector< Matrix > > model_instances;

	for ( auto& i : get_component_list() )
	{
		auto* transform = std::get< TransformComponent* >( i.second );
		auto* model_component = std::get< ModelComponent* >( i.second );

		// 距離カリング
		const Vector& entity_pos = transform->transform.get_position();
		const Vector to_entity = entity_pos - camera_position;
		const float distance_sq = to_entity.x() * to_entity.x() + to_entity.y() * to_entity.y() + to_entity.z() * to_entity.z();

		if ( distance_sq > culling_distance_sq )
		{
			continue; // カリング距離を超えているのでスキップ
		}

		// 背面カリング (カメラの後ろにあるオブジェクトをスキップ)
		const float dot = to_entity.x() * camera_forward.x() + to_entity.y() * camera_forward.y() + to_entity.z() * camera_forward.z();
		if ( dot < -50.f ) // 少し余裕を持たせる (50m 後ろまでは描画)
		{
			continue;
		}

		// ワールド行列を計算
		Matrix world;
		world.set_identity();
		world *= Matrix().set_rotation_quaternion( transform->transform.get_rotation() );
		world *= Matrix().set_translation( transform->transform.get_position() );

		// モデルごとにグループ化
		model_instances[ model_component->model ].push_back( world );
	}

	// インスタンシングシェーダーを取得
	auto* instanced_shader = get_graphics_manager()->get_shader( "lit_instanced" );

	// 各モデルグループを描画
	for ( auto& [ model, matrices ] : model_instances )
	{
		if ( matrices.empty() ) continue;

		const size_t instance_count = matrices.size();

		if ( instancing_enabled_ && instance_count > 1 && instanced_shader )
		{
			// インスタンシング描画
			instance_buffer_->update( matrices.data(), instance_count );

			for ( uint_t n = 0; n < model->get_shader_count(); n++ )
			{
				auto* shader = model->get_shader_at( n );

				// テクスチャをバインド
				if ( shader->get_texture_at( 0 ) )
				{
					instanced_shader->set_texture( shader->get_texture_at( 0 ) );
				}

				// インスタンシングシェーダーを使用して描画
				instanced_shader->render_instanced( model->get_mesh(), n, static_cast< uint_t >( instance_count ), instance_buffer_.get() );
			}
		}
		else
		{
			// 通常描画 (1インスタンスずつ)
			for ( const auto& world : matrices )
			{
				core::ObjectConstantBufferWithData shader_data;
				shader_data.data().world = world;
				shader_data.update();

				get_graphics_manager()->set_current_object_constant_buffer( & shader_data );
				get_graphics_manager()->set_current_skinning_constant_buffer( nullptr );
				model->render();
			}
		}
	}

	core::graphics::Shader* beat_pulse_shader = nullptr;

	if ( is_post_effect_enabled )
	{
		beat_pulse_shader = get_graphics_manager()->get_shader( "post_effect_beat_pulse" );

		get_graphics_manager()->set_post_effect_shader( beat_pulse_shader );
		get_graphics_manager()->render_post_effect( render_result_texture_1_.get() ); // 直接バックバッファへ


		/*
		get_graphics_manager()->set_post_effect_shader( get_graphics_manager()->get_shader( "post_effect_hand_drawing" ) );
		get_graphics_manager()->render_post_effect( render_result_texture_2_.get() );
		*/

		// get_graphics_manager()->set_post_effect_shader( get_graphics_manager()->get_shader( "post_effect_id_to_color" ) );
		// get_graphics_manager()->render_post_effect( render_result_texture_1_.get(), render_result_texture_2_.get() );

		// get_graphics_manager()->set_post_effect_shader( get_graphics_manager()->get_shader( "post_effect_hand_drawing" ) );

		// ノイズシェーダーは無効化
		// noise_shader->set_texture_at( 1, get_graphics_manager()->load_texture( "media/texture/noise.png" ) );
		// get_graphics_manager()->set_post_effect_shader( noise_shader );
		// get_graphics_manager()->render_post_effect( render_result_texture_2_.get() );
	}

	// get_graphics_manager()->render_fader();
	// get_graphics_manager()->render_debug_bullet();
	
	// ImGUI によるシェーダーパラメター変更用 UI の自動描画
	noise_shader->render_parameter_gui();

	if ( is_post_effect_enabled )
	{
		beat_pulse_shader->render_parameter_gui();
	}
}

} // namespace core::ecs
