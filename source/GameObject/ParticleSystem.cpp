#include "ParticleSystem.h"

namespace blue_sky
{

ParticleSystem::ParticleSystem()
	: constant_buffer_( sizeof( Vertex ) * particle_list_.size() )
{

}

ParticleSystem::~ParticleSystem()
{

}

void ParticleSystem::update()
{
	for ( auto& p : particle_list_)
	{
		p.Position.y() += 0.01f;
	}
}

void ParticleSystem::update_render_data() const
{
	ActiveObject::update_render_data();
	constant_buffer_.update( & particle_list_ );
}

void ParticleSystem::bind_render_data() const
{
	ActiveObject::bind_render_data();

	constant_buffer_.bind_to_vs();
}

}
