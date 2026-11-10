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
	// 以前は生の new / delete。宣言の逆の順に壊れる（世界 → 解く物 → 広い判定 → 振り分け → 設定。以前の delete と同じ順）
	std::unique_ptr< btDefaultCollisionConfiguration > collision_configuration;
	std::unique_ptr< btCollisionDispatcher > collision_dispatcher;
	std::unique_ptr< btBroadphaseInterface > broadphase_interface;
	std::unique_ptr< btSequentialImpulseConstraintSolver > constraint_solver;
	std::unique_ptr< btDiscreteDynamicsWorld > dynamics_world;

	float time_step = 1.f / 60.f;
	math::Vector gravity = math::Vector( 0.f, -9.8f, 0.f );

	Impl()
	{
		collision_configuration = std::make_unique< btDefaultCollisionConfiguration >();
		collision_dispatcher = std::make_unique< btCollisionDispatcher >( collision_configuration.get() );
		broadphase_interface = std::make_unique< btDbvtBroadphase >();
		constraint_solver = std::make_unique< btSequentialImpulseConstraintSolver >();
		dynamics_world = std::make_unique< btDiscreteDynamicsWorld >(
			collision_dispatcher.get(),
			broadphase_interface.get(),
			constraint_solver.get(),
			collision_configuration.get()
		);

		dynamics_world->setGravity( btVector3( gravity.x(), gravity.y(), gravity.z() ) );
	}

	/**
	 * 世界を壊す（unique_ptr が宣言の逆の順に壊す）
	 *
	 * 剛体は持ち主（RigidBody。部品が持つ）が壊れるときに世界から外して消すので、ここでは消さない。
	 * 以前はここで世界に残った剛体・形・動きの状態を全部消していた（今は持ち主がこれより先に壊れる決まり。
	 * 残っていると、持ち主が後で壊れるときに消えた世界から外そうとする）
	 */
	~Impl() = default;
};

PhysicsManager::PhysicsManager()
	: impl_( std::make_unique< Impl >() )
{
}

PhysicsManager::~PhysicsManager() = default;

RigidBodyHandle PhysicsManager::create_rigid_body( const RigidBodyCreateInfo& info )
{
	// CollisionShape を生成（この剛体だけの形。持ち主が一緒に消す）
	const auto& size = info.shape_size;
	std::unique_ptr< btCollisionShape > collision_shape;

	switch ( info.shape_type )
	{
	case RigidBodyShapeType::Box:
		collision_shape = std::make_unique< btBoxShape >( btVector3( size.x(), size.y(), size.z() ) );
		break;

	case RigidBodyShapeType::Capsule:
		collision_shape = std::make_unique< btCapsuleShape >( size.x(), size.y() );
		break;

	case RigidBodyShapeType::Cylinder:
		collision_shape = std::make_unique< btCylinderShape >( btVector3( size.x(), size.y(), size.z() ) );
		break;

	case RigidBodyShapeType::Sphere:
		collision_shape = std::make_unique< btSphereShape >( size.x() );
		break;

	default:
		collision_shape = std::make_unique< btBoxShape >( btVector3( size.x(), size.y(), size.z() ) );
		break;
	}

	// 慣性モーメントを計算
	btVector3 local_inertia( 0, 0, 0 );
	if ( info.mass > 0.f )
	{
		collision_shape->calculateLocalInertia( info.mass, local_inertia );
	}

	// Transform を btTransform に変換
	btTransform bt_transform = info.transform;

	// MotionState を生成
	auto motion_state = std::make_unique< btDefaultMotionState >( bt_transform );

	// RigidBody を生成
	btRigidBody::btRigidBodyConstructionInfo rigid_body_info(
		info.mass,
		motion_state.get(),
		collision_shape.get(),
		local_inertia
	);

	// 摩擦は全物体で 0.1、反発は Bullet の既定。物体ごとに変えたいときは RigidBodyComponent に摩擦・反発を足し、ここで rigid_body_info に渡す
	auto rigid_body = std::make_unique< btRigidBody >( rigid_body_info );
	rigid_body->setFriction( 0.1f );
	// rigid_body->setActivationState( DISABLE_DEACTIVATION );

	// user_pointer は付けない（Entity の番号のタグは使う側が付ける。core/physics/EntityTag.h）
	rigid_body->setUserPointer( nullptr );

	// dynamics_world に登録 (衝突グループ/マスク付き)
	impl_->dynamics_world->addRigidBody( rigid_body.get(), info.collision_group, info.collision_mask );

	return RigidBodyHandle( impl_->dynamics_world.get(), std::move( rigid_body ), std::move( motion_state ), std::move( collision_shape ) );
}

void PhysicsManager::destroy_rigid_body( RigidBodyHandle& handle )
{
	// 世界から外して、剛体・動きの状態・形を消す（以前はここで 3 つを delete していた）
	handle.reset();
}

int PhysicsManager::get_rigid_body_count() const
{
	return impl_->dynamics_world ? impl_->dynamics_world->getNumCollisionObjects() : 0;
}

void PhysicsManager::get_rigid_body_transform( const RigidBodyHandle& handle, Vector& position, math::Quaternion& rotation ) const
{
	if ( ! handle.is_valid() )
	{
		return;
	}

	btTransform bt_transform;
	handle->getMotionState()->getWorldTransform( bt_transform );

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

void PhysicsManager::set_rigid_body_linear_velocity( const RigidBodyHandle& handle, const Vector& velocity )
{
	if ( ! handle.is_valid() )
	{
		return;
	}

	handle->setLinearVelocity( btVector3( velocity.x(), velocity.y(), velocity.z() ) );
}

PhysicsManager::Vector PhysicsManager::get_rigid_body_linear_velocity( const RigidBodyHandle& handle ) const
{
	if ( ! handle.is_valid() )
	{
		return Vector( 0.f, 0.f, 0.f );
	}

	const btVector3& v = handle->getLinearVelocity();
	return Vector( v.x(), v.y(), v.z() );
}

void PhysicsManager::set_rigid_body_angular_factor( const RigidBodyHandle& handle, const Vector& factor )
{
	if ( ! handle.is_valid() )
	{
		return;
	}

	handle->setAngularFactor( btVector3( factor.x(), factor.y(), factor.z() ) );
}

void PhysicsManager::apply_impulse( const RigidBodyHandle& handle, const Vector& impulse )
{
	if ( ! handle.is_valid() )
	{
		return;
	}

	handle->applyCentralImpulse( btVector3( impulse.x(), impulse.y(), impulse.z() ) );
}

void PhysicsManager::activate_rigid_body( const RigidBodyHandle& handle )
{
	if ( ! handle.is_valid() )
	{
		return;
	}

	handle->activate( true );
}

bool PhysicsManager::ray_test( const Vector& from, const Vector& to ) const
{
	btVector3 bt_from( from.x(), from.y(), from.z() );
	btVector3 bt_to( to.x(), to.y(), to.z() );

	btCollisionWorld::ClosestRayResultCallback ray_callback( bt_from, bt_to );
	impl_->dynamics_world->rayTest( bt_from, bt_to, ray_callback );

	return ray_callback.hasHit();
}

bool PhysicsManager::ray_test_excluding( const Vector& from, const Vector& to, const RigidBodyHandle& exclude ) const
{
	btVector3 bt_from( from.x(), from.y(), from.z() );
	btVector3 bt_to( to.x(), to.y(), to.z() );

	btCollisionWorld::ClosestRayResultCallback ray_callback( bt_from, bt_to );

	// 除外する剛体を設定
	if ( exclude.is_valid() )
	{
		ray_callback.m_collisionFilterMask &= ~exclude->getBroadphaseProxy()->m_collisionFilterGroup;
	}

	impl_->dynamics_world->rayTest( bt_from, bt_to, ray_callback );

	// ヒットしたオブジェクトが除外対象でないか確認
	if ( ray_callback.hasHit() && exclude.is_valid() )
	{
		if ( ray_callback.m_collisionObject == exclude.get() )
		{
			return false;
		}
	}

	return ray_callback.hasHit();
}

bool PhysicsManager::check_contact( const RigidBodyHandle& a, const RigidBodyHandle& b ) const
{
	if ( ! a.is_valid() || ! b.is_valid() )
	{
		return false;
	}

	// 全てのコンタクトマニフォールドを走査
	const int num_manifolds = impl_->dynamics_world->getDispatcher()->getNumManifolds();

	for ( int i = 0; i < num_manifolds; i++ )
	{
		btPersistentManifold* manifold = impl_->dynamics_world->getDispatcher()->getManifoldByIndexInternal( i );

		const btCollisionObject* body0 = manifold->getBody0();
		const btCollisionObject* body1 = manifold->getBody1();

		// a と b のペアかどうかを確認
		bool is_pair = ( body0 == a.get() && body1 == b.get() ) ||
		               ( body0 == b.get() && body1 == a.get() );

		if ( is_pair && manifold->getNumContacts() > 0 )
		{
			return true;
		}
	}

	return false;
}

void PhysicsManager::for_each_contact( short group_a, short group_b,
	const std::function< void( void*, void* ) >& callback ) const
{
	const int num_manifolds = impl_->dynamics_world->getDispatcher()->getNumManifolds();

	for ( int i = 0; i < num_manifolds; i++ )
	{
		btPersistentManifold* manifold = impl_->dynamics_world->getDispatcher()->getManifoldByIndexInternal( i );

		if ( manifold->getNumContacts() == 0 )
		{
			continue;
		}

		const btCollisionObject* body0 = manifold->getBody0();
		const btCollisionObject* body1 = manifold->getBody1();

		const short group0 = static_cast< short >( body0->getBroadphaseHandle()->m_collisionFilterGroup );
		const short group1 = static_cast< short >( body1->getBroadphaseHandle()->m_collisionFilterGroup );

		// group_a と group_b のペアかどうかを確認
		void* ptr_a = nullptr;
		void* ptr_b = nullptr;

		if ( ( group0 & group_a ) && ( group1 & group_b ) )
		{
			ptr_a = body0->getUserPointer();
			ptr_b = body1->getUserPointer();
		}
		else if ( ( group0 & group_b ) && ( group1 & group_a ) )
		{
			ptr_a = body1->getUserPointer();
			ptr_b = body0->getUserPointer();
		}
		else
		{
			continue;
		}

		if ( ptr_a && ptr_b )
		{
			callback( ptr_a, ptr_b );
		}
	}
}

} // namespace core::physics
