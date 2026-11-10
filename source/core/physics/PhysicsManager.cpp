#include "PhysicsManager.h"
#include "EntityTag.h"
#include <core/math/Quaternion.h>
#include <common/exception.h>
#include <btBulletDynamicsCommon.h>

#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <tuple>

namespace core::physics
{

namespace
{

/// 重力の既定（地球。以前の BulletPhysics と PhysicsManager で同じ値）
const math::Vector DefaultGravity( 0.f, -9.8f, 0.f );

} // namespace

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
	math::Vector gravity = DefaultGravity;

	/// 形の種類と大きさ → 共有する形（以前の BulletPhysics::shape_cache_。share_shape の剛体が使う）
	using ShapeKey = std::tuple< int, btScalar, btScalar, btScalar >;
	std::map< ShapeKey, std::unique_ptr< btCollisionShape > > shared_shape_map;

	/// ステージの当たりの形（load_obj）の形と三角形（剛体より後に壊す）
	std::vector< std::unique_ptr< btCollisionShape > > mesh_shape_list;
	std::vector< std::unique_ptr< btTriangleMesh > > mesh_list;

	/// これが持つ剛体（地面・ステージの当たりの形。形より先に壊す）
	std::vector< RigidBody > owned_rigid_body_list;

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
	~Impl()
	{
		clear();
	}

	/// これが持つ剛体・形・制約を消し、残っている剛体を世界から外す（以前の BulletPhysics::clear と同じ順）
	void clear()
	{
		for ( int n = dynamics_world->getNumConstraints() - 1; n >= 0; n-- )
		{
			btTypedConstraint* constraint = dynamics_world->getConstraint( n );
			dynamics_world->removeConstraint( constraint );
			delete constraint;
		}

		// これが持つ剛体（地面・ステージの当たりの形）を消す（持ち主が世界から外す）
		owned_rigid_body_list.clear();

		// 残っているのは物体の部品が持つ剛体。消すと持ち主が後で二重に消すので、世界から外すだけにする
		for ( int n = dynamics_world->getNumCollisionObjects() - 1; n >= 0; n-- )
		{
			dynamics_world->removeCollisionObject( dynamics_world->getCollisionObjectArray()[ n ] );
		}

		shared_shape_map.clear();
		mesh_shape_list.clear();
		mesh_list.clear();
	}

	/// 同じ種類・大きさの形があれば返し、無ければ作って登録する（以前の BulletPhysics::find_or_create_shape）
	btCollisionShape* find_or_create_shared_shape( const ShapeKey& key, const std::function< std::unique_ptr< btCollisionShape >() >& make )
	{
		auto i = shared_shape_map.find( key );

		if ( i != shared_shape_map.end() )
		{
			return i->second.get();
		}

		return shared_shape_map.emplace( key, make() ).first->second.get();
	}
};

namespace
{

/// 形を作る（RigidBodyCreateInfo の種類と大きさから）
std::unique_ptr< btCollisionShape > make_shape( RigidBodyShapeType type, const math::Vector& size )
{
	switch ( type )
	{
	case RigidBodyShapeType::Capsule:
		return std::make_unique< btCapsuleShape >( size.x(), size.y() );

	case RigidBodyShapeType::Cylinder:
		return std::make_unique< btCylinderShape >( btVector3( size.x(), size.y(), size.z() ) );

	case RigidBodyShapeType::Sphere:
		return std::make_unique< btSphereShape >( size.x() );

	case RigidBodyShapeType::Box:
	default:
		return std::make_unique< btBoxShape >( btVector3( size.x(), size.y(), size.z() ) );
	}
}

/// 自分（と相手）を除き、filter が当たると決めた剛体だけに当たるレイ（以前の blue_sky::ClosestNotMe・ClosestNotMeAndHim）
class FilteredClosestRayResultCallback : public btCollisionWorld::ClosestRayResultCallback
{
private:
	const btCollisionObject* exclude_a_;
	const btCollisionObject* exclude_b_;
	const BodyFilter& filter_;

public:
	FilteredClosestRayResultCallback( const btVector3& from, const btVector3& to, const btCollisionObject* exclude_a, const btCollisionObject* exclude_b, const BodyFilter& filter )
		: ClosestRayResultCallback( from, to )
		, exclude_a_( exclude_a )
		, exclude_b_( exclude_b )
		, filter_( filter )
	{ }

	btScalar addSingleResult( btCollisionWorld::LocalRayResult& ray_result, bool normal_in_world_space ) override
	{
		if ( ray_result.m_collisionObject == exclude_b_ || ray_result.m_collisionObject == exclude_a_ )
		{
			return 1.0;
		}

		if ( filter_ && ! filter_( get_entity_tag( ray_result.m_collisionObject ) ) )
		{
			return 1.0;
		}

		return ClosestRayResultCallback::addSingleResult( ray_result, normal_in_world_space );
	}
};

/// 当たりがあったかだけを調べる
class HitContactResultCallback : public btCollisionWorld::ContactResultCallback
{
public:
	bool is_hit = false;

	btScalar addSingleResult( btManifoldPoint&, const btCollisionObjectWrapper*, int, int, const btCollisionObjectWrapper*, int, int ) override
	{
		is_hit = true;

		return 0;
	}
};

/// 接した組のタグを渡す
class TagContactResultCallback : public btCollisionWorld::ContactResultCallback
{
private:
	const ContactCallback& callback_;

public:
	explicit TagContactResultCallback( const ContactCallback& callback ) : callback_( callback ) { }

	btScalar addSingleResult( btManifoldPoint&, const btCollisionObjectWrapper* o0, int, int, const btCollisionObjectWrapper* o1, int, int ) override
	{
		callback_( get_entity_tag( o0->getCollisionObject() ), get_entity_tag( o1->getCollisionObject() ) );

		return 0;
	}
};

/// 自分以外の、filter が数えると決めた物に触れたか（以前のプレイヤーの WithoutMeContactResultCallback）
class FilteredContactResultCallback : public btCollisionWorld::ContactResultCallback
{
private:
	const btCollisionObject* exclude_;
	const BodyFilter& filter_;

	bool is_ignored( const btCollisionObject* object ) const
	{
		if ( object == exclude_ )
		{
			return true;
		}

		return filter_ && ! filter_( get_entity_tag( object ) );
	}

public:
	bool is_hit = false;

	FilteredContactResultCallback( const btCollisionObject* exclude, const BodyFilter& filter )
		: exclude_( exclude )
		, filter_( filter )
	{ }

	btScalar addSingleResult( btManifoldPoint&, const btCollisionObjectWrapper* o1, int, int, const btCollisionObjectWrapper* o2, int, int ) override
	{
		if ( is_ignored( o1->getCollisionObject() ) || is_ignored( o2->getCollisionObject() ) )
		{
			return 1.f;
		}

		is_hit = true;

		return 1.f;
	}
};

} // namespace

PhysicsManager::PhysicsManager()
	: impl_( std::make_unique< Impl >() )
{
}

PhysicsManager::~PhysicsManager() = default;

RigidBodyHandle PhysicsManager::create_rigid_body( const RigidBodyCreateInfo& info )
{
	// CollisionShape を生成（共有しないなら、この剛体だけの形。持ち主が一緒に消す）
	const auto& size = info.shape_size;
	std::unique_ptr< btCollisionShape > owned_shape;
	btCollisionShape* collision_shape = nullptr;

	if ( info.share_shape )
	{
		const Impl::ShapeKey key( static_cast< int >( info.shape_type ), size.x(), size.y(), size.z() );
		collision_shape = impl_->find_or_create_shared_shape( key, [ & ] { return make_shape( info.shape_type, size ); } );
	}
	else
	{
		owned_shape = make_shape( info.shape_type, size );
		collision_shape = owned_shape.get();
	}

	// 慣性モーメントを計算（質量 0 の形は Bullet の計算でも 0 になる。以前の BulletPhysics は質量によらず計算していた）
	btVector3 local_inertia( 0, 0, 0 );
	if ( info.mass > 0.f )
	{
		collision_shape->calculateLocalInertia( info.mass, local_inertia );
	}

	// Transform を btTransform に変換（ずれがあれば右から掛ける。以前の BulletPhysics::create_rigid_body と同じ計算）
	btTransform bt_transform = info.transform;

	if ( info.offset )
	{
		bt_transform = bt_transform * btTransform( * info.offset );
	}

	// MotionState を生成
	auto motion_state = std::make_unique< btDefaultMotionState >( bt_transform );

	// RigidBody を生成
	btRigidBody::btRigidBodyConstructionInfo rigid_body_info(
		info.mass,
		motion_state.get(),
		collision_shape,
		local_inertia
	);

	// 反発は Bullet の既定
	auto rigid_body = std::make_unique< btRigidBody >( rigid_body_info );
	rigid_body->setFriction( info.friction );
	// rigid_body->setActivationState( DISABLE_DEACTIVATION );

	// user_pointer は付けない（Entity の番号のタグは使う側が付ける。RigidBody::set_entity_tag）
	rigid_body->setUserPointer( nullptr );

	if ( info.use_default_collision_filter )
	{
		// Bullet の既定の振り分け（以前の BulletPhysics と同じ）
		impl_->dynamics_world->addRigidBody( rigid_body.get() );
	}
	else
	{
		// dynamics_world に登録 (衝突グループ/マスク付き)
		impl_->dynamics_world->addRigidBody( rigid_body.get(), info.collision_group, info.collision_mask );
	}

	return RigidBodyHandle( impl_->dynamics_world.get(), std::move( rigid_body ), std::move( motion_state ), std::move( owned_shape ) );
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
	handle.get()->getMotionState()->getWorldTransform( bt_transform );

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

	handle.get()->setLinearVelocity( btVector3( velocity.x(), velocity.y(), velocity.z() ) );
}

PhysicsManager::Vector PhysicsManager::get_rigid_body_linear_velocity( const RigidBodyHandle& handle ) const
{
	if ( ! handle.is_valid() )
	{
		return Vector( 0.f, 0.f, 0.f );
	}

	const btVector3& v = handle.get()->getLinearVelocity();
	return Vector( v.x(), v.y(), v.z() );
}

void PhysicsManager::set_rigid_body_angular_factor( const RigidBodyHandle& handle, const Vector& factor )
{
	if ( ! handle.is_valid() )
	{
		return;
	}

	handle.get()->setAngularFactor( btVector3( factor.x(), factor.y(), factor.z() ) );
}

void PhysicsManager::apply_impulse( const RigidBodyHandle& handle, const Vector& impulse )
{
	if ( ! handle.is_valid() )
	{
		return;
	}

	handle.get()->applyCentralImpulse( btVector3( impulse.x(), impulse.y(), impulse.z() ) );
}

void PhysicsManager::activate_rigid_body( const RigidBodyHandle& handle )
{
	if ( ! handle.is_valid() )
	{
		return;
	}

	handle.get()->activate( true );
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
		ray_callback.m_collisionFilterMask &= ~exclude.get()->getBroadphaseProxy()->m_collisionFilterGroup;
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
	const std::function< void( ecs::EntityId, ecs::EntityId ) >& callback ) const
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
		const btCollisionObject* object_a = nullptr;
		const btCollisionObject* object_b = nullptr;

		if ( ( group0 & group_a ) && ( group1 & group_b ) )
		{
			object_a = body0;
			object_b = body1;
		}
		else if ( ( group0 & group_b ) && ( group1 & group_a ) )
		{
			object_a = body1;
			object_b = body0;
		}
		else
		{
			continue;
		}

		// 両方に Entity の番号のタグがある組だけ（以前は userPointer が両方 nullptr でないかを見ていた）
		const auto id_a = get_entity_tag( object_a );
		const auto id_b = get_entity_tag( object_b );

		if ( id_a && id_b )
		{
			callback( * id_a, * id_b );
		}
	}
}

void PhysicsManager::clear()
{
	impl_->clear();
	set_gravity( DefaultGravity );
}

void PhysicsManager::add_ground( const Vector& half_extents )
{
	// 以前の BulletPhysics::add_ground_rigid_body と同じ（形は共有・質量 0・Bullet の既定の摩擦と振り分け・ずれは単位）
	RigidBodyCreateInfo info;
	info.shape_type = RigidBodyShapeType::Box;
	info.shape_size = half_extents;
	info.mass = 0.f;
	info.transform.set_identity();
	info.transform.set_position( Vector( 0, -half_extents.y(), 0 ) );
	info.offset = Transform::identity();
	info.friction = RigidBodyCreateInfo::BulletDefaultFriction;
	info.share_shape = true;
	info.use_default_collision_filter = true;

	impl_->owned_rigid_body_list.push_back( create_rigid_body( info ) );
}

bool PhysicsManager::load_obj( const char_t* file_name )
{
	std::ifstream in( file_name );

	if ( ! in.good() )
	{
		return false;
	}

	auto triangle = std::make_unique< btTriangleMesh >();
	std::vector< btVector3 > vs;

	while ( in.good() )
	{
		std::string line;
		std::getline( in, line );

		std::stringstream ss;
		std::string command;

		ss << line;
		ss >> command;

		if ( command == "v" )
		{
			btScalar x, y, z;

			ss >> x >> y >> z;

			vs.push_back( btVector3( x, y, -z ) );
		}
		else if ( command == "f" )
		{
			std::vector< uint_t > ns;

			while ( ss.good() )
			{
				uint_t n;

				ss >> n;

				// 1 origin -> 0 origin
				n--;

				ns.push_back( n );
			}

			if ( ns.size() == 3 )
			{
				triangle->addTriangle( vs[ ns[ 0 ] ], vs[ ns[ 1 ] ], vs[ ns[ 2 ] ], true );
			}
		}
	}

	if ( triangle->getNumTriangles() <= 0 )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( string_t( "empty triangles : " ) + file_name );
	}

	auto shape = std::make_unique< btBvhTriangleMeshShape >( triangle.get(), true );

	btTransform transform;
	transform.setIdentity();

	// 静的な剛体なので慣性は不要
	btVector3 local_inertia( 0, 0, 0 );

	auto motion_state = std::make_unique< btDefaultMotionState >( transform );
	btRigidBody::btRigidBodyConstructionInfo rigid_body_info( 0.f, motion_state.get(), shape.get(), local_inertia );

	auto rigid_body = std::make_unique< btRigidBody >( rigid_body_info );
	impl_->dynamics_world->addRigidBody( rigid_body.get() );

	// ステージの当たりの形はこれが持つ（clear で消える。形と三角形は剛体より後に壊す）
	impl_->mesh_list.push_back( std::move( triangle ) );
	impl_->mesh_shape_list.push_back( std::move( shape ) );
	impl_->owned_rigid_body_list.emplace_back( impl_->dynamics_world.get(), std::move( rigid_body ), std::move( motion_state ) );

	return true;
}

RayHit PhysicsManager::ray_test_closest( const Vector& from, const Vector& to, const RigidBody* exclude_a, const RigidBody* exclude_b, const BodyFilter& filter ) const
{
	const btVector3 bt_from( from.x(), from.y(), from.z() );
	const btVector3 bt_to( to.x(), to.y(), to.z() );

	FilteredClosestRayResultCallback callback( bt_from, bt_to, exclude_a ? exclude_a->get() : nullptr, exclude_b ? exclude_b->get() : nullptr, filter );
	callback.m_closestHitFraction = 1.0;

	impl_->dynamics_world->rayTest( bt_from, bt_to, callback );

	RayHit hit;
	hit.hit = callback.hasHit();
	hit.fraction = callback.m_closestHitFraction;

	if ( hit.hit )
	{
		hit.point = Vector( callback.m_hitPointWorld.x(), callback.m_hitPointWorld.y(), callback.m_hitPointWorld.z() );
		hit.normal = Vector( callback.m_hitNormalWorld.x(), callback.m_hitNormalWorld.y(), callback.m_hitNormalWorld.z() );
		hit.entity = get_entity_tag( callback.m_collisionObject );
	}

	return hit;
}

bool PhysicsManager::contact_pair_test( const RigidBody* a, const RigidBody* b ) const
{
	if ( ! a || ! b || ! a->is_valid() || ! b->is_valid() )
	{
		return false;
	}

	HitContactResultCallback callback;
	impl_->dynamics_world->contactPairTest( a->get(), b->get(), callback );

	return callback.is_hit;
}

void PhysicsManager::contact_test( const RigidBody& body, const ContactCallback& callback ) const
{
	if ( ! body.is_valid() )
	{
		return;
	}

	TagContactResultCallback result( callback );
	impl_->dynamics_world->contactTest( body.get(), result );
}

void PhysicsManager::for_each_penetrating_contact( const ContactCallback& callback ) const
{
	const int num_manifolds = impl_->dynamics_world->getDispatcher()->getNumManifolds();

	for ( int i = 0; i < num_manifolds; i++ )
	{
		const btPersistentManifold* manifold = impl_->dynamics_world->getDispatcher()->getManifoldByIndexInternal( i );
		const btCollisionObject* o0 = manifold->getBody0();
		const btCollisionObject* o1 = manifold->getBody1();

		const int num_contacts = manifold->getNumContacts();

		for ( int j = 0; j < num_contacts; j++ )
		{
			if ( manifold->getContactPoint( j ).getDistance() < 0.f )
			{
				callback( get_entity_tag( o0 ), get_entity_tag( o1 ) );
			}
		}
	}
}

bool PhysicsManager::box_contact_test( const Transform& transform, const Vector& offset_position, const Vector& half_extents, const RigidBody* exclude, const BodyFilter& filter ) const
{
	btBoxShape shape( btVector3( half_extents.x(), half_extents.y(), half_extents.z() ) );

	btTransform offset;
	offset.setIdentity();
	offset.setOrigin( btVector3( offset_position.x(), offset_position.y(), offset_position.z() ) );

	btCollisionObject collision_object;
	collision_object.setCollisionShape( & shape );
	collision_object.setWorldTransform( btTransform( transform ) * offset );

	FilteredContactResultCallback callback( exclude ? exclude->get() : nullptr, filter );
	impl_->dynamics_world->contactTest( & collision_object, callback );

	return callback.is_hit;
}

PhysicsManager::Vector PhysicsManager::get_box_half_extents_with_margin( const Vector& half_extents )
{
	const btVector3 v = btBoxShape( btVector3( half_extents.x(), half_extents.y(), half_extents.z() ) ).getHalfExtentsWithMargin();

	return Vector( v.x(), v.y(), v.z() );
}

std::size_t PhysicsManager::get_owned_shape_count() const
{
	return impl_->shared_shape_map.size() + impl_->mesh_shape_list.size();
}

std::size_t PhysicsManager::get_owned_rigid_body_count() const
{
	return impl_->owned_rigid_body_list.size();
}

} // namespace core::physics
