#include "EntityObject.h"

#include <blue_sky/ecs/ObjectBody.h>
#include <blue_sky/ecs/component/PhysicsBodyComponent.h>
#include <blue_sky/ecs/component/SimpleObjectComponent.h>
#include <blue_sky/ecs/component/CollisionTraitsComponent.h>
#include <blue_sky/ecs/component/ObjectRenderComponent.h>
#include <blue_sky/ecs/component/PickupComponent.h>
#include <blue_sky/ecs/component/ContactCacheComponent.h>
#include <blue_sky/ecs/system/PhysicsSyncSystem.h>

#include <core/ecs/Entity.h>
#include <core/ecs/EntityManager.h>
#include <core/ecs/component/TransformComponent.h>
#include <core/math/Quaternion.h>
#include <core/sound/Sound.h>
#include <core/sound/SoundManager.h>

#include <common/math.h>

#include <btBulletDynamicsCommon.h>

#include <cmath>

namespace blue_sky::ecs
{

EntityObject::EntityObject( core::ecs::Entity* entity )
	: entity_( entity )
	, transform_( entity ? entity->get_component< core::ecs::TransformComponent >() : nullptr )
	, body_( entity ? entity->get_component< PhysicsBodyComponent >() : nullptr )
	, state_( entity ? entity->get_component< SimpleObjectComponent >() : nullptr )
{

}

btRigidBody* EntityObject::get_rigid_body() const
{
	return body_ ? body_->body : nullptr;
}

void EntityObject::commit_transform()
{
	if ( get_rigid_body() )
	{
		PhysicsSyncSystem::write_body( * get_rigid_body(), body_->height_offset, transform_->transform );
	}
}

const core::Vector& EntityObject::get_location() const
{
	return transform_->transform.get_position();
}

void EntityObject::set_location( const core::Vector& location )
{
	transform_->transform.set_position( location );
	commit_transform();
}

core::Vector EntityObject::get_velocity() const
{
	return get_rigid_body() ? object_body::get_velocity( * get_rigid_body() ) : core::Vector::Zero;
}

void EntityObject::set_velocity( const core::Vector& velocity )
{
	if ( get_rigid_body() )
	{
		object_body::set_velocity( * get_rigid_body(), velocity );
	}
}

void EntityObject::update_velocity_by_target_location( const core::Vector& target_location, float_t speed )
{
	core::Vector relative_position = target_location - get_location();

	if ( relative_position.length() > 0.1f )
	{
		relative_position.normalize();
	}

	set_velocity( relative_position * speed );
}

void EntityObject::stop_at_target_location( const core::Vector& target_location )
{
	set_location( target_location );
	set_velocity( core::Vector::Zero );
}

float_t EntityObject::get_direction_degree() const
{
	return state_ ? state_->direction_degree : 0.f;
}

void EntityObject::set_direction_degree( float_t degree )
{
	while ( degree < 0.f ) degree += 360.f;
	while ( degree > 360.f ) degree -= 360.f;

	if ( state_ )
	{
		state_->direction_degree = degree;
	}

	core::Quaternion q{};
	q.set_yaw_pitch_roll( math::degree_to_radian( degree ), 0.f, 0.f );
	transform_->transform.set_rotation( q );

	commit_transform();
}

void EntityObject::chase_direction_degree( float_t d, float_t speed )
{
	float_t current = get_direction_degree();

	while ( current < 0.f ) current += 360.f;
	while ( current > 360.f ) current -= 360.f;

	while ( d < 0.f ) d += 360.f;
	while ( d > 360.f ) d -= 360.f;

	float_t diff = d - current;

	while ( diff < -180.f ) diff += 360.f;
	while ( diff >  180.f ) diff -= 360.f;

	set_direction_degree( math::chase( current, current + diff, speed ) );
}

void EntityObject::chase_direction_to( const core::Vector& location, float_t speed )
{
	core::Vector relative_position = location - get_location();
	relative_position.set_y( 0 );

	chase_direction_degree( math::radian_to_degree( std::atan2( relative_position.x(), relative_position.z() ) ), speed );
}

void EntityObject::set_start_location( const core::Vector& location )
{
	if ( state_ )
	{
		state_->start_location = location;
	}

	// 剛体へは送らない（以前の ActiveObject::set_start_location と同じ。剛体を作る前に呼ぶ・やり直しで送る）
	transform_->transform.set_position( location );
}

void EntityObject::set_start_rotation( float_t x, float_t y, float_t z )
{
	if ( state_ )
	{
		state_->start_rotation.set( x, y, z );
	}

	core::Quaternion q{};
	q.set_yaw_pitch_roll( math::degree_to_radian( y ), math::degree_to_radian( x ), math::degree_to_radian( z ) );
	transform_->transform.set_rotation( q );
}

void EntityObject::set_start_direction_degree( float_t degree )
{
	if ( state_ )
	{
		state_->start_direction_degree = degree;
	}
}

void EntityObject::restart()
{
	if ( auto* traits = entity_->get_component< CollisionTraitsComponent >() )
	{
		traits->is_dead = false;
	}

	if ( btRigidBody* body = get_rigid_body() )
	{
		const core::Vector start_location = state_ ? state_->start_location : get_location();
		const core::Vector start_rotation = state_ ? state_->start_rotation : core::Vector::Zero;

		core::Transform& transform = transform_->transform;

		transform.set_position( start_location );
		transform.set_rotation( core::Quaternion::from_yaw_pitch_roll( math::degree_to_radian( start_rotation.y() ), math::degree_to_radian( start_rotation.x() ), math::degree_to_radian( start_rotation.z() ) ) );

		// 以前の ActiveObject::restart と同じ順・同じ値（動きの状態・世界の位置には足元の位置を入れ、最後に向きを変えるときに高さのずれを足して送り直す）
		body->activate( true );
		body->getMotionState()->setWorldTransform( transform );

		body->setWorldTransform( transform );
		body->setInterpolationWorldTransform( transform );

		body->setLinearVelocity( btVector3( 0.f, 0.f, 0.f ) );
		body->setInterpolationLinearVelocity( btVector3( 0.f, 0.f, 0.f ) );

		body->setAngularVelocity( btVector3( 0.f, 0.f, 0.f ) );
		body->setInterpolationAngularVelocity( btVector3( 0.f, 0.f, 0.f ) );

		body->setGravity( state_ ? state_->default_gravity : core::Vector::Zero );

		body->clearForces();

		set_direction_degree( state_ ? state_->start_direction_degree : 0.f );

		// 種類ごとのやり直し（以前は派生クラスの restart）
		if ( state_ && state_->restart_mass >= 0.f )
		{
			object_body::set_mass( * body, state_->restart_mass );		// 以前の Goal::restart
		}

		if ( const auto* pickup = entity_->get_component< PickupComponent >(); pickup && pickup->disable_contact_on_pickup )
		{
			object_body::set_no_contact_response( * body, false );		// 以前の Rocket::restart（拾われたときに消した当たりの応答を戻す）
		}

		if ( const auto* throwable = entity_->get_component< ThrowableComponent >() )
		{
			// 以前の Stone::restart（石が薄い壁を貫通しないようにする。Bullet の連続衝突判定）
			body->setCcdSweptSphereRadius( throwable->ccd_swept_sphere_radius );
			body->setCcdMotionThreshold( throwable->ccd_motion_threshold );
		}
	}

	// 最近触れた物体を忘れる（以前の Ladder::restart・BaseSwitch::restart）
	if ( auto* cache = entity_->get_component< ContactCacheComponent >() )
	{
		cache->cache.clear();
	}

	set_visible( true, true );
}

void EntityObject::kill()
{
	if ( auto* traits = entity_->get_component< CollisionTraitsComponent >() )
	{
		traits->is_dead = true;
	}

	set_visible( false, false );
}

bool EntityObject::is_dead() const
{
	const auto* traits = entity_->get_component< CollisionTraitsComponent >();

	return traits && traits->is_dead;
}

void EntityObject::play_sound( core::sound::SoundManager* sound_manager, const char_t* name ) const
{
	core::Sound* sound = sound_manager ? sound_manager->get_sound( name ) : nullptr;

	if ( ! sound )
	{
		return;
	}

	// 以前の GameObject::play_sound と同じ値を渡す（速度の 2 つ目に z を渡しているのも以前のまま）
	const core::Vector location = get_location();
	const core::Vector velocity = get_velocity();

	sound->set_3d_position( location.x(), location.y(), location.z() );
	sound->set_3d_velocity( velocity.x(), velocity.z(), velocity.z() );
	sound->play( false );
}

void EntityObject::set_visible( bool mesh, bool line )
{
	if ( auto* render = entity_->get_component< ObjectRenderComponent >() )
	{
		render->is_mesh_visible = mesh;
		render->is_line_visible = line;
		render->is_visible = mesh || line;
	}
}

} // namespace blue_sky::ecs
