#include "ParticleRenderSystem.h"
#include <core/ConstantBuffers.h>
#include <core/graphics/VertexBuffer.h>
#include <core/graphics/GraphicsManager.h>
#include <iostream>


namespace core::ecs
{


ParticleRenderSystem::ParticleRenderSystem( core::graphics::GraphicsManager* graphics_manager, std::unique_ptr< core::graphics::Shader > shader )
	: graphics_manager_( graphics_manager )
	, shader_( std::move( shader ) )
	, object_constant_buffer_( graphics_manager->create_constant_buffer( sizeof( core::ObjectConstantBufferData ), core::ObjectConstantBuffer::SLOT, core::graphics::ConstantBufferUsage::DYNAMIC ) )	// 描画ごとに書き換える
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

		// 頂点バッファは GraphicsManager が作る（以前は Direct3D11 を受け取って自分で作っていた。その前は関数内の static で、中で Direct3D11 の get_instance() を使っていた）
		//
		// 頂点バッファは 1 つで、全 Entity が使い回す。粒子の一覧の大きさが型で決まっている（std::array）ので、
		// 最初の Entity で作った大きさが全 Entity に合う。Entity ごとに WRITE_DISCARD で書いてすぐ描く（render_technique は
		// その場で呼ぶ）ので、粒子の Entity が 2 つ以上でも前の Entity の描画は壊れない（2026-11-10 に確かめた）。
		// 一覧を可変長（vector など）にするなら、頂点バッファを Entity ごとにするか、最大数に合わせて作り直すこと
		static_assert( std::tuple_size< ParticleSystemComponent::ParticleList >::value > 0, "粒子の一覧は大きさが型で決まる配列であること（上の説明）" );

		if ( ! vertex_buffer_ )
		{
			vertex_buffer_ = get_graphics_manager()->create_dynamic_vertex_buffer( sizeof( ParticleSystemComponent::ParticleList::value_type ), particle_system->particle_list.size() );
		}

		vertex_buffer_->update( particle_system->particle_list.data(), particle_system->particle_list.size() );

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
			vertex_buffer_->draw( particle_system->particle_list.size() );
		} );
	}

	shader_->render_parameter_gui();
}

} // namespace core::ecs
