#include "RenderSystem.h"
#include <blue_sky/ConstantBuffers.h> /// @todo core ���� blue_sky ���Q�Ƃ��Ă���͕̂ςȂ̂ŁA��{�I�� ConstatntBuffer �� core �Ɉڂ�
#include <blue_sky/GameMain.h> /// @todo core ���� blue_sky ���Q�Ƃ��Ă���͕̂ςȂ̂ŁA����
#include <blue_sky/graphics/GraphicsManager.h> /// @todo core ���� blue_sky ���Q�Ƃ��Ă���͕̂ςȂ̂ŁA����
#include <iostream>

namespace core::ecs
{

void RenderSystem::update()
{
	// std::cout << get_priority() << " : ParticleRenderSystem::update()" << std::endl;

	for ( auto& i : get_component_list() )
	{
		auto* transform = std::get< TransformComponent* >( i.second );
		// std::cout << "postion : " << transform->transform.get_position() << std::endl;

		/// @todo Transform �� Direct3D �� update ����
		blue_sky::ObjectConstantBufferWithData shader_data;

		shader_data.data().world.set_identity();
		shader_data.data().world.set_rotation_quaternion( transform->transform.get_rotation() );
		shader_data.data().world *= Matrix().set_translation( transform->transform.get_position() );
		shader_data.data().world = shader_data.data().world.transpose();

		shader_data.update();

		blue_sky::GameMain::get_instance()->get_graphics_manager()->set_current_object_constant_buffer( & shader_data );
		blue_sky::GameMain::get_instance()->get_graphics_manager()->set_current_skinning_constant_buffer( nullptr );
		blue_sky::GameMain::get_instance()->get_graphics_manager()->get_model( "wall-1" )->render();
	}
}

} // namespace core::ecs
