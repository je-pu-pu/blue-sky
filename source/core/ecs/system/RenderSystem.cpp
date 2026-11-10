#include "RenderSystem.h"
#include <core/ConstantBuffers.h>
#include <core/TimeManager.h>
#include <core/graphics/GraphicsManager.h>
#include <core/graphics/Model.h>
#include <core/graphics/Shader.h>
#include <core/graphics/RenderTargetTexture.h>

#include <common/math.h>

#include <iostream>
#include <unordered_map>

namespace core::ecs
{

RenderSystem::RenderSystem( core::graphics::GraphicsManager* graphics_manager, const core::TimeManager* time_manager )
	: graphics_manager_( graphics_manager )
	, time_manager_( time_manager )
	// , render_result_texture_1_( get_graphics_manager()->create_render_target_texture( core::graphics::PixelFormat::R8_UINT ) )
	, render_result_texture_1_( get_graphics_manager()->create_render_target_texture() )
	, render_result_texture_2_( get_graphics_manager()->create_render_target_texture() )
	, instance_buffer_( get_graphics_manager()->create_instance_buffer() )
	, object_constant_buffer_( get_graphics_manager()->create_constant_buffer( sizeof( core::ObjectConstantBufferData ), core::ObjectConstantBuffer::SLOT, core::graphics::ConstantBufferUsage::DYNAMIC ) )	// 描画ごとに書き換える
	, noise_shader_( get_graphics_manager()->get_shader_handle( "post_effect_noise" ) )
	, instanced_shader_( get_graphics_manager()->get_shader_handle( "lit_instanced" ) )
	, beat_pulse_shader_( get_graphics_manager()->get_shader_handle( "post_effect_beat_pulse" ) )
{
	if ( noise_shader_.is_null() || beat_pulse_shader_.is_null() )
	{
		// 以前は毎フレーム引いて、無ければ落ちていた。今は無いものを使わずに描く
		std::cout << "[RenderSystem] ポストエフェクトのシェーダーがありません（setup_default_shaders の前に作った）。ポストエフェクトを使わずに描きます" << std::endl;	// core は debug_log を知らない。GameMain が debug_log へ流す
	}
}

core::graphics::Shader* RenderSystem::resolve_shader( core::graphics::ShaderHandle h )
{
	auto* shader = get_graphics_manager()->resolve_shader( h );

	if ( ! shader && ! h.is_null() && ! shader_lost_logged_ )
	{
		shader_lost_logged_ = true;
		std::cout << "[RenderSystem] 使うシェーダーが解放されました。そのシェーダーを使わずに描きます" << std::endl;	// core は debug_log を知らない。GameMain が debug_log へ流す
	}

	return shader;
}

/**
 * 前のフレームの world 行列を消す（容量は残す）
 *
 * 前のフレームで使わなかったモデルは map から外す（使わなくなったモデルの番号が溜まり続けないように）
 */
void RenderSystem::clear_model_instances()
{
	for ( auto i = model_instances_.begin(); i != model_instances_.end(); )
	{
		if ( i->second.empty() )
		{
			i = model_instances_.erase( i );
		}
		else
		{
			i->second.clear();
			++i;
		}
	}
}

void RenderSystem::update()
{
	auto* noise_shader = resolve_shader( noise_shader_ );

	if ( noise_shader )
	{
		noise_shader->set_float( "offset", time_manager_->get_total_elapsed_time() );
	}

	get_graphics_manager()->setup_rendering();

	core::graphics::Shader* beat_pulse_shader = resolve_shader( beat_pulse_shader_ );
	const auto is_post_effect_enabled = beat_pulse_shader != nullptr;	// 無ければ画面へ直に描く（テクスチャに描いて出さない、にならないように）

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
	clear_model_instances();

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
		model_instances_[ model_component->model ].push_back( world );
	}

	auto* instanced_shader = resolve_shader( instanced_shader_ );

	// 各モデルグループを描画
	for ( auto& [ model_handle, matrices ] : model_instances_ )
	{
		if ( matrices.empty() ) continue;

		// モデルはハンドルから引く（空・解放済みなら描かない。モデルごとに 1 回）
		core::graphics::Model* model = model_handle.get();

		if ( ! model )
		{
			if ( ! model_handle.is_null() && ! model_lost_logged_ )
			{
				model_lost_logged_ = true;
				std::cout << "[RenderSystem] 描くモデルが解放されました。そのモデルの物体は描きません" << std::endl;
			}

			continue;
		}

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
				core::ObjectConstantBufferData shader_data;
				shader_data.world = world;
				object_constant_buffer_->update( & shader_data );

				get_graphics_manager()->set_current_object_constant_buffer( object_constant_buffer_.get() );
				get_graphics_manager()->set_current_skinning_constant_buffer( nullptr );
				model->render();
			}
		}
	}

	if ( is_post_effect_enabled )
	{
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
	if ( noise_shader )
	{
		noise_shader->render_parameter_gui();
	}

	if ( beat_pulse_shader )
	{
		beat_pulse_shader->render_parameter_gui();
	}
}

} // namespace core::ecs
