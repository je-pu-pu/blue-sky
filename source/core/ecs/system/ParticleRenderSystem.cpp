#include "ParticleRenderSystem.h"
#include <core/ConstantBuffers.h>
#include <core/graphics/VertexBuffer.h>
#include <core/graphics/GraphicsManager.h>
#include <core/graphics/Direct3D11/Direct3D11.h>
#include <iostream>


namespace core::ecs
{

using core::graphics::BufferType;
using core::graphics::VertexBuffer;
using core::graphics::direct_3d_11::Direct3D11;

ParticleRenderSystem::ParticleRenderSystem( core::graphics::GraphicsManager* graphics_manager, core::graphics::direct_3d_11::Direct3D11* direct_3d, std::unique_ptr< core::graphics::Shader > shader )
	: graphics_manager_( graphics_manager )
	, direct_3d_( direct_3d )
	, shader_( std::move( shader ) )
	, object_constant_buffer_( graphics_manager->create_constant_buffer( sizeof( core::ObjectConstantBufferData ), core::ObjectConstantBuffer::SLOT ) )
{
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

		// 頂点バッファは Direct3D11 を GraphicsManager から受け取って作る（以前は関数内の static で、中で Direct3D11 の get_instance() を使っていた）
		if ( ! vertex_buffer_ )
		{
			vertex_buffer_ = std::make_unique< VertexBuffer< ParticleSystemComponent::ParticleList > >( direct_3d_, particle_system->particle_list, BufferType::UPDATABLE );
		}

		vertex_buffer_->update( particle_system->particle_list );

		// Constant Buffer
		core::ObjectConstantBufferData object_data;
		object_data.world.set_identity();
		object_data.color = Color::White;
		object_constant_buffer_->update( & object_data );

		get_graphics_manager()->set_input_layout( "pos_norm" );
		get_graphics_manager()->set_primitive_topology( core::graphics::PrimitiveTopology::POINT_LIST );
		get_graphics_manager()->render_technique( "point_sprite", [this, particle_system] {

			object_constant_buffer_->bind_to_vs();
			object_constant_buffer_->bind_to_gs();
			shader_->bind();
			vertex_buffer_->bind();

			// Draw
			direct_3d_->getImmediateContext()->Draw( particle_system->particle_list.size(), 0 );
		} );
	}

	shader_->render_parameter_gui();
}

} // namespace core::ecs
