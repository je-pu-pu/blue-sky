#include "ParticleRenderSystem.h"
#include <blue_sky/graphics/shader/PointSpriteShader.h> /// @todo core に移動
#include <blue_sky/ConstantBuffers.h> /// @todo core に移動
#include <core/Service.h>
#include <core/graphics/VertexBuffer.h>
#include <core/graphics/GraphicsManager.h>
#include <core/graphics/Direct3D11/Direct3D11.h>
#include <iostream>


namespace core::ecs
{

using core::graphics::BufferType;
using core::graphics::VertexBuffer;
using core::graphics::direct_3d_11::Direct3D11;

ParticleRenderSystem::ParticleRenderSystem()
	: shader_( std::make_unique< blue_sky::graphics::shader::PointSpriteShader >() )
{
	// shader_->set_texture_at( 0, get_graphics_manager()->load_texture( "media/texture/pen/white-grass-pen.png" ) );
	shader_->set_texture_at( 0, get_graphics_manager()->load_texture( "media/texture/pen/circle.png" ) );
}

void ParticleRenderSystem::update()
{
	// std::cout << get_priority() << " : ParticleRenderSystem::update()" << std::endl;

	// get_graphics_manager()->setup_rendering();
	// get_graphics_manager()->set_default_render_target();

	// テストとして位置情報を標準出力へ出力
	for ( auto& i : get_component_list() )
	{
		// auto* transform = std::get< TransformComponent* >( i.second );
		// std::cout << "postion : " << transform->transform.get_position() << std::endl;

		auto* particle_system = std::get< ParticleSystemComponent* >( i.second );
		
		// 全てのパーティクルの位置を出力する
		/*
		for ( const auto& p : particle_system->particle_list )
		{
			// 遅いのでコメントアウト
			std::cout << "\tp : " << p.position << std::endl;
		}
		*/

		// const auto& p = particle_system->particle_list.front();
		// std::cout << "p0 pos, vel : " << p.position << ", " <<  p.velocity << std::endl;

		static auto vb = VertexBuffer{ particle_system->particle_list, BufferType::UPDATABLE };

		vb.update( particle_system->particle_list );

		// Constant Buffer
		static blue_sky::ObjectConstantBufferWithData object_data;
		object_data.data().world.set_identity();
		object_data.data().color = Color::White;
		object_data.update();

		get_graphics_manager()->set_input_layout( "pos_norm" );
		get_graphics_manager()->set_primitive_topology( core::graphics::PrimitiveTopology::POINT_LIST );
		get_graphics_manager()->render_technique( "point_sprite", [=] {

			object_data.bind_to_vs();
			object_data.bind_to_gs();
			shader_->bind();
			vb.bind();

			// Draw
			Direct3D11::get_instance()->getImmediateContext()->Draw( particle_system->particle_list.size(), 0 );
		} );
	}

	shader_->render_parameter_gui();
}

} // namespace core::ecs
