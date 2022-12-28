#include "RenderSystem.h"
#include <blue_sky/ConstantBuffers.h> /// @todo core から blue_sky を参照しているのは変なので、基本的な ConstatntBuffer は core に移す
#include <core/graphics/GraphicsManager.h>
#include <core/graphics/Model.h>
#include <core/graphics/RenderTargetTexture.h>

namespace core::ecs
{

RenderSystem::RenderSystem()
	: render_result_texture_1_( get_graphics_manager()->create_render_target_texture( core::graphics::PixelFormat::R8_UINT ) )
	, render_result_texture_2_( get_graphics_manager()->create_render_target_texture() )
{
	//
}

void RenderSystem::update()
{
	get_graphics_manager()->setup_rendering();

	render_result_texture_1_->clear();
	get_graphics_manager()->set_render_target( render_result_texture_1_.get() );

	// get_graphics_manager()->render_background();

	for ( auto& i : get_component_list() )
	{
		auto* transform = std::get< TransformComponent* >( i.second );
		auto* model = std::get< ModelComponent* >( i.second );

		/// Transform を Shader に update する
		blue_sky::ObjectConstantBufferWithData shader_data;

		shader_data.data().world.set_identity();
		shader_data.data().world.set_rotation_quaternion( transform->transform.get_rotation() );
		shader_data.data().world *= Matrix().set_translation( transform->transform.get_position() );
		shader_data.data().world = shader_data.data().world.transpose();

		shader_data.update();

		get_graphics_manager()->set_current_object_constant_buffer( & shader_data );
		get_graphics_manager()->set_current_skinning_constant_buffer( nullptr );
		model->model->render();
	}

	/*
	get_graphics_manager()->set_post_effect_shader( get_graphics_manager()->get_shader( "post_effect_chromatic_aberrration" ) );
	get_graphics_manager()->render_post_effect( render_result_texture_1_.get(), render_result_texture_2_.get() );

	get_graphics_manager()->set_post_effect_shader( get_graphics_manager()->get_shader( "post_effect_hand_drawing" ) );
	get_graphics_manager()->render_post_effect( render_result_texture_2_.get() );
	*/

	get_graphics_manager()->set_post_effect_shader( get_graphics_manager()->get_shader( "post_effect_id_to_color" ) );
	get_graphics_manager()->render_post_effect( render_result_texture_1_.get(), render_result_texture_2_.get() );

	// get_graphics_manager()->set_post_effect_shader( get_graphics_manager()->get_shader( "post_effect_hand_drawing" ) );
	get_graphics_manager()->set_post_effect_shader( get_graphics_manager()->get_shader( "post_effect_noise" ) );
	get_graphics_manager()->render_post_effect( render_result_texture_2_.get() );

	get_graphics_manager()->set_default_render_target();

	// get_graphics_manager()->render_fader();
	// get_graphics_manager()->render_debug_bullet();
	
	// ImGUI によるシェーダーパラメター変更用 UI の自動描画
	auto* shader = get_graphics_manager()->get_shader( "post_effect_noise" );
	shader->render_parameter_gui();
}

} // namespace core::ecs
