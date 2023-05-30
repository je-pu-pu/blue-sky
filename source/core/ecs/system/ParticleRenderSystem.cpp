#include "ParticleRenderSystem.h"
#include <iostream>

namespace core::ecs
{

void ParticleRenderSystem::update()
{
	// std::cout << get_priority() << " : ParticleRenderSystem::update()" << std::endl;

	// テストとして位置情報を標準出力へ出力
	for ( auto& i : get_component_list() )
	{
		auto* transform = std::get< TransformComponent* >( i.second );
		std::cout << "postion : " << transform->transform.get_position() << std::endl;

		auto* particle_system = std::get< ParticleSystemComponent* >( i.second );
		
		// 全てのパーティクルの位置を出力する
		for ( const auto& p : particle_system->particle_list )
		{
			// 遅いのでコメントアウト
			std::cout << "\tp : " << p.position << std::endl;
		}
	}
}

} // namespace core::ecs
