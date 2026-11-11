#include "RigidBody.h"
#include "EntityTag.h"

#include <core/math/Quaternion.h>

#include <btBulletDynamicsCommon.h>

#include <utility>

namespace core::physics
{

namespace
{

/// btVector3 と Vector はメモリレイアウトが同一（以前の blue_sky の ObjectBody・PhysicsSyncSystem と同じ変換）
const math::Vector& to_vector( const btVector3& v )
{
	return reinterpret_cast< const math::Vector& >( v );
}

} // namespace

RigidBody::RigidBody() = default;

RigidBody::RigidBody( btDynamicsWorld* world, std::unique_ptr< btRigidBody > rigid_body, std::unique_ptr< btMotionState > motion_state, std::unique_ptr< btCollisionShape > owned_shape )
	: world_( world )
	, rigid_body_( std::move( rigid_body ) )
	, motion_state_( std::move( motion_state ) )
	, owned_shape_( std::move( owned_shape ) )
{

}

RigidBody::~RigidBody()
{
	reset();
}

RigidBody::RigidBody( RigidBody&& other ) noexcept
	: world_( std::exchange( other.world_, nullptr ) )
	, rigid_body_( std::move( other.rigid_body_ ) )
	, motion_state_( std::move( other.motion_state_ ) )
	, owned_shape_( std::move( other.owned_shape_ ) )
{

}

RigidBody& RigidBody::operator = ( RigidBody&& other ) noexcept
{
	if ( this != & other )
	{
		reset();

		world_ = std::exchange( other.world_, nullptr );
		rigid_body_ = std::move( other.rigid_body_ );
		motion_state_ = std::move( other.motion_state_ );
		owned_shape_ = std::move( other.owned_shape_ );
	}

	return * this;
}

void RigidBody::reset()
{
	if ( rigid_body_ && world_ )
	{
		world_->removeRigidBody( rigid_body_.get() );
	}

	// 剛体 → 動きの状態 → 形 の順に壊す（剛体が後の 2 つを指しているので）
	rigid_body_.reset();
	motion_state_.reset();
	owned_shape_.reset();
	world_ = nullptr;
}

RigidBody::Transform RigidBody::get_world_transform( const Vector& local_offset ) const
{
	btTransform t = rigid_body_->getWorldTransform();

	btTransform offset;
	offset.setIdentity();
	offset.setOrigin( local_offset );

	t *= offset;

	Transform transform;
	transform.set_position( to_vector( t.getOrigin() ) );

	const btQuaternion q = t.getRotation();
	transform.set_rotation( reinterpret_cast< const math::Quaternion& >( q ) );

	return transform;
}

void RigidBody::set_world_transform( const Transform& transform, const Vector& local_offset )
{
	btTransform offset;
	offset.setIdentity();
	offset.setOrigin( local_offset );

	set_world_transform_raw( btTransform( transform ) * offset );
}

void RigidBody::set_world_transform( const Transform& transform )
{
	set_world_transform_raw( transform );
}

void RigidBody::set_world_transform_raw( const btTransform& t )
{
	rigid_body_->setWorldTransform( t );
	rigid_body_->setInterpolationWorldTransform( t );

	// キネマティックな剛体は、Bullet が step のたびに動きの状態（motion state）から位置を読み直す
	// （btRigidBody::saveKinematicState）。動きの状態にも書かないと、書いた位置が元に戻される
	if ( rigid_body_->isKinematicObject() && rigid_body_->getMotionState() )
	{
		rigid_body_->getMotionState()->setWorldTransform( t );
	}
}

void RigidBody::set_motion_state_transform( const Transform& transform )
{
	rigid_body_->getMotionState()->setWorldTransform( transform );
}

RigidBody::Vector RigidBody::get_center_of_mass_position() const
{
	// 以前のプレイヤーの足場の判定と同じく x・y・z から作る（4 つ目の値は持ち込まない）
	const btVector3& c = rigid_body_->getCenterOfMassPosition();

	return Vector( c.x(), c.y(), c.z() );
}

RigidBody::Vector RigidBody::get_linear_velocity() const
{
	return to_vector( rigid_body_->getLinearVelocity() );
}

void RigidBody::set_linear_velocity( const Vector& v )
{
	rigid_body_->setLinearVelocity( v );
}

void RigidBody::set_interpolation_linear_velocity( const Vector& v )
{
	rigid_body_->setInterpolationLinearVelocity( v );
}

void RigidBody::set_angular_velocity( const Vector& v )
{
	rigid_body_->setAngularVelocity( v );
}

void RigidBody::set_interpolation_angular_velocity( const Vector& v )
{
	rigid_body_->setInterpolationAngularVelocity( v );
}

void RigidBody::clear_forces()
{
	rigid_body_->clearForces();
}

void RigidBody::activate( bool force_activation )
{
	rigid_body_->activate( force_activation );
}

void RigidBody::set_activation_state_active()
{
	rigid_body_->setActivationState( ACTIVE_TAG );
}

void RigidBody::set_mass( float_t mass )
{
	btVector3 local_inertia( 0, 0, 0 );
	rigid_body_->getCollisionShape()->calculateLocalInertia( mass, local_inertia );
	rigid_body_->setMassProps( mass, local_inertia );
}

float_t RigidBody::get_inv_mass() const
{
	return rigid_body_->getInvMass();
}

void RigidBody::set_gravity( const Vector& gravity )
{
	rigid_body_->setGravity( gravity );
}

RigidBody::Vector RigidBody::get_gravity() const
{
	return to_vector( rigid_body_->getGravity() );
}

void RigidBody::set_friction( float_t friction )
{
	rigid_body_->setFriction( friction );
}

float_t RigidBody::get_friction() const
{
	return rigid_body_->getFriction();
}

void RigidBody::set_angular_factor( float_t factor )
{
	rigid_body_->setAngularFactor( factor );
}

void RigidBody::set_angular_factor( const Vector& factor )
{
	rigid_body_->setAngularFactor( factor );
}

RigidBody::Vector RigidBody::get_angular_factor() const
{
	return to_vector( rigid_body_->getAngularFactor() );
}

void RigidBody::set_kinematic( bool is_kinematic )
{
	if ( is_kinematic )
	{
		rigid_body_->setCollisionFlags( rigid_body_->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT );
	}
	else
	{
		rigid_body_->setCollisionFlags( rigid_body_->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT );
	}
}

bool RigidBody::is_kinematic() const
{
	return rigid_body_->isKinematicObject();
}

void RigidBody::set_no_contact_response( bool is_no_contact_response )
{
	if ( is_no_contact_response )
	{
		rigid_body_->setCollisionFlags( rigid_body_->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE );
	}
	else
	{
		rigid_body_->setCollisionFlags( rigid_body_->getCollisionFlags() & ~btCollisionObject::CF_NO_CONTACT_RESPONSE );
	}
}

bool RigidBody::has_contact_response() const
{
	return rigid_body_->hasContactResponse();
}

void RigidBody::set_ccd( float_t swept_sphere_radius, float_t motion_threshold )
{
	rigid_body_->setCcdSweptSphereRadius( swept_sphere_radius );
	rigid_body_->setCcdMotionThreshold( motion_threshold );
}

float_t RigidBody::get_ccd_swept_sphere_radius() const
{
	return rigid_body_->getCcdSweptSphereRadius();
}

float_t RigidBody::get_ccd_motion_threshold() const
{
	return rigid_body_->getCcdMotionThreshold();
}

RigidBodyShapeType RigidBody::get_shape_type() const
{
	switch ( rigid_body_->getCollisionShape()->getShapeType() )
	{
	case BOX_SHAPE_PROXYTYPE:					return RigidBodyShapeType::Box;
	case CAPSULE_SHAPE_PROXYTYPE:				return RigidBodyShapeType::Capsule;
	case CYLINDER_SHAPE_PROXYTYPE:				return RigidBodyShapeType::Cylinder;
	case SPHERE_SHAPE_PROXYTYPE:				return RigidBodyShapeType::Sphere;
	case TRIANGLE_MESH_SHAPE_PROXYTYPE:			return RigidBodyShapeType::TriangleMesh;
	default:									return RigidBodyShapeType::Other;
	}
}

void RigidBody::set_entity_tag( ecs::EntityId id )
{
	core::physics::set_entity_tag( rigid_body_.get(), id );
}

void RigidBody::clear_entity_tag()
{
	core::physics::clear_entity_tag( rigid_body_.get() );
}

std::optional< ecs::EntityId > RigidBody::get_entity_tag() const
{
	return core::physics::get_entity_tag( rigid_body_.get() );
}

} // namespace core::physics
