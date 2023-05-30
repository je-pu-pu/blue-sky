#include "ParticleRenderSystem.h"
#include <iostream>

namespace core::ecs
{

void ParticleRenderSystem::update()
{
	// std::cout << get_priority() << " : ParticleRenderSystem::update()" << std::endl;

	// �e�X�g�Ƃ��Ĉʒu����W���o�͂֏o��
	for ( auto& i : get_component_list() )
	{
		auto* transform = std::get< TransformComponent* >( i.second );
		std::cout << "postion : " << transform->transform.get_position() << std::endl;

		auto* particle_system = std::get< ParticleSystemComponent* >( i.second );
		
		// �S�Ẵp�[�e�B�N���̈ʒu���o�͂���
		for ( const auto& p : particle_system->particle_list )
		{
			// �x���̂ŃR�����g�A�E�g
			std::cout << "\tp : " << p.position << std::endl;
		}
	}
}

} // namespace core::ecs
