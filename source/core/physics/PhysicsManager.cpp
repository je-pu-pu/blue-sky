#include "PhysicsManager.h"
#include <core/math/Quaternion.h>
#include <btBulletDynamicsCommon.h>

namespace core::physics
{

/**
 * PhysicsManager の実装クラス (Pimpl)
 */
class PhysicsManager::Impl
{
public:
	btDefaultCollisionConfiguration* collision_configuration = nullptr;
	btCollisionDispatcher* collision_dispatcher = nullptr;
	btBroadphaseInterface* broadphase_interface = nullptr;
	btSequentialImpulseConstraintSolver* constraint_solver = nullptr;
	btDiscreteDynamicsWorld* dynamics_world = nullptr;

	float time_step = 1.f / 60.f;
	math::Vector gravity = math::Vector( 0.f, -9.8f, 0.f );

	Impl()
	{
		collision_configuration = new btDefaultCollisionConfiguration();
		collision_dispatcher = new btCollisionDispatcher( collision_configuration );
		broadphase_interface = new btDbvtBroadphase();
		constraint_solver = new btSequentialImpulseConstraintSolver();
		dynamics_world = new btDiscreteDynamicsWorld(
			collision_dispatcher,
			broadphase_interface,
			constraint_solver,
			collision_configuration
		);

		dynamics_world->setGravity( btVector3( gravity.x(), gravity.y(), gravity.z() ) );
	}

	~Impl()
	{
		// dynamics_world に残っている剛体を全て削除する
		if ( dynamics_world )
		{
			for ( int i = dynamics_world->getNumCollisionObjects() - 1; i >= 0; i-- )
			{
				btCollisionObject* obj = dynamics_world->getCollisionObjectArray()[ i ];
				btRigidBody* body = btRigidBody::upcast( obj );

				if ( body )
				{
					// MotionState を削除
					if ( body->getMotionState() )
					{
						delete body->getMotionState();
					}

					// CollisionShape を削除
					if ( body->getCollisionShape() )
					{
						delete body->getCollisionShape();
					}

					// ワールドから削除
					dynamics_world->removeRigidBody( body );
				}
				else
				{
					dynamics_world->removeCollisionObject( obj );
				}

				// オブジェクトを削除
				delete obj;
			}
		}

		delete dynamics_world;
		delete constraint_solver;
		delete broadphase_interface;
		delete collision_dispatcher;
		delete collision_configuration;
	}
};

PhysicsManager::PhysicsManager()
	: impl_( new Impl() )
{
}

PhysicsManager::~PhysicsManager()
{
	delete impl_;
}

RigidBodyHandle PhysicsManager::create_rigid_body( const RigidBodyCreateInfo& info )
{
	RigidBodyHandle handle;

	// CollisionShape を生成
	const auto& size = info.shape_size;

	switch ( info.shape_type )
	{
	case RigidBodyShapeType::Box:
		handle.collision_shape = new btBoxShape( btVector3( size.x(), size.y(), size.z() ) );
		break;

	case RigidBodyShapeType::Capsule:
		handle.collision_shape = new btCapsuleShape( size.x(), size.y() );
		break;

	case RigidBodyShapeType::Cylinder:
		handle.collision_shape = new btCylinderShape( btVector3( size.x(), size.y(), size.z() ) );
		break;

	case RigidBodyShapeType::Sphere:
		handle.collision_shape = new btSphereShape( size.x() );
		break;

	default:
		handle.collision_shape = new btBoxShape( btVector3( size.x(), size.y(), size.z() ) );
		break;
	}

	// 慣性モーメントを計算
	btVector3 local_inertia( 0, 0, 0 );
	if ( info.mass > 0.f )
	{
		handle.collision_shape->calculateLocalInertia( info.mass, local_inertia );
	}

	// Transform を btTransform に変換
	btTransform bt_transform = info.transform;

	// MotionState を生成
	handle.motion_state = new btDefaultMotionState( bt_transform );

	// RigidBody を生成
	btRigidBody::btRigidBodyConstructionInfo rigid_body_info(
		info.mass,
		handle.motion_state,
		handle.collision_shape,
		local_inertia
	);

	// rigid_body_info.m_friction = 0.01f;
	// rigid_body_info.m_restitution = 0.5f;

	handle.rigid_body = new btRigidBody( rigid_body_info );
	handle.rigid_body->setFriction( 0.1f );
	// handle.rigid_body->setActivationState( DISABLE_DEACTIVATION );

	// dynamics_world に登録
	impl_->dynamics_world->addRigidBody( handle.rigid_body );

	return handle;
}

void PhysicsManager::destroy_rigid_body( RigidBodyHandle& handle )
{
	if ( ! handle.is_valid() )
	{
		return;
	}

	// dynamics_world から削除
	if ( impl_->dynamics_world && handle.rigid_body )
	{
		impl_->dynamics_world->removeRigidBody( handle.rigid_body );
	}

	// リソースを解放
	delete handle.rigid_body;
	handle.rigid_body = nullptr;

	delete handle.motion_state;
	handle.motion_state = nullptr;

	delete handle.collision_shape;
	handle.collision_shape = nullptr;
}

void PhysicsManager::get_rigid_body_transform( const RigidBodyHandle& handle, Vector& position, math::Quaternion& rotation ) const
{
	if ( ! handle.is_valid() )
	{
		return;
	}

	btTransform bt_transform;
	handle.rigid_body->getMotionState()->getWorldTransform( bt_transform );

	const btVector3& origin = bt_transform.getOrigin();
	position = Vector( origin.x(), origin.y(), origin.z() );

	const btQuaternion& q = bt_transform.getRotation();
	rotation = reinterpret_cast<const math::Quaternion&>( q );
}

void PhysicsManager::step_simulation()
{
	step_simulation( impl_->time_step );
}

void PhysicsManager::step_simulation( float time_step )
{
	impl_->dynamics_world->stepSimulation( time_step, 5, 1.f / 120.f );
	impl_->dynamics_world->debugDrawWorld();
}

void PhysicsManager::set_gravity( const Vector& gravity )
{
	impl_->gravity = gravity;
	impl_->dynamics_world->setGravity( btVector3( gravity.x(), gravity.y(), gravity.z() ) );
}

PhysicsManager::Vector PhysicsManager::get_gravity() const
{
	return impl_->gravity;
}

void PhysicsManager::set_time_step( float time_step )
{
	impl_->time_step = time_step;
}

float PhysicsManager::get_time_step() const
{
	return impl_->time_step;
}

void PhysicsManager::set_debug_drawer( btIDebugDraw* drawer )
{
	impl_->dynamics_world->setDebugDrawer( drawer );
}

btDynamicsWorld* PhysicsManager::get_dynamics_world()
{
	return impl_->dynamics_world;
}

} // namespace core::physics
