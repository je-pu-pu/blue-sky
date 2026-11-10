#include "RigidBody.h"

#include <btBulletDynamicsCommon.h>

#include <utility>

namespace core::physics
{

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

} // namespace core::physics
