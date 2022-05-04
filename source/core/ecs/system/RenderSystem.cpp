#include "RenderSystem.h"
#include <blue_sky/ConstantBuffers.h> /// @todo core から blue_sky を参照しているのは変なので、基本的な ConstatntBuffer は core に移す
#include <blue_sky/GameMain.h> /// @todo core から blue_sky を参照しているのは変なので、直す
#include <blue_sky/graphics/GraphicsManager.h> /// @todo core から blue_sky を参照しているのは変なので、直す
#include <core/graphics/Model.h>
#include <iostream>

namespace core::ecs
{

void RenderSystem::update()
{
	get_graphics_manager()->setup_rendering();

	get_graphics_manager()->render_background();

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

	get_graphics_manager()->render_fader();
	get_graphics_manager()->render_debug_bullet();
}

} // namespace core::ecs
