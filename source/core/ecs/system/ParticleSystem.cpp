#include "ParticleSystem.h"
#include <core/math/Quaternion.h>
#include <common/random.h>
#include <common/math.h>

#include <iostream>

namespace core::ecs
{

using namespace ::math::literals;

void ParticleSystem::update( ComponentTuple& component_tuple ) const
{
	for ( auto& p : std::get< ParticleSystemComponent* >( component_tuple )->particle_list )
	{
		p.position += p.velocity;
		p.velocity.y() -= 9.80665f / 60.f * 0.001f;

		if ( p.position.y() < 0.f )
		{
			p.position.set( 0.f, 0.f, 0.f );

			float yaw = ::common::random( -180._deg, 180._deg );
			float pitch = ::common::random( 0._deg, 45._deg );

			const auto v = Vector::transform( Vector::Up, Quaternion::from_yaw_pitch_roll( yaw, pitch, 0.f ) ) * common::random( 0.1f, 0.2f );

			p.velocity.set( v.x(), v.y(), v.z() );
		}
	}
}

} // namespace core::ecs
