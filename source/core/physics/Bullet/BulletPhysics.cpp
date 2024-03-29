#include "BulletPhysics.h"
#include <common/exception.h>

#include <vector>
#include <fstream>
#include <sstream>

BulletPhysics::BulletPhysics()
	: collision_configuration_( 0 )
	, collision_dispatcher_( 0 )
	, broadphase_interface_( 0 )
	, constraint_solver_( 0 )
	, dynamics_world_( 0 )
{
	collision_configuration_ = new btDefaultCollisionConfiguration();
	collision_dispatcher_ = new btCollisionDispatcher( collision_configuration_ );
	broadphase_interface_ = new btDbvtBroadphase();
	constraint_solver_ = new btSequentialImpulseConstraintSolver();
	dynamics_world_ = new btDiscreteDynamicsWorld( collision_dispatcher_, broadphase_interface_, constraint_solver_, collision_configuration_ );

	dynamics_world_->setGravity( btVector3( 0, -9.8f, 0 ) );
 }

BulletPhysics::~BulletPhysics()
{
	clear();

	delete dynamics_world_;
	delete constraint_solver_;
	delete broadphase_interface_;
	delete collision_dispatcher_;
	delete collision_configuration_;
}

void BulletPhysics::clear()
{
	for ( int n = dynamics_world_->getNumConstraints() - 1; n >= 0; n-- )
	{
		btTypedConstraint* constraint = dynamics_world_->getConstraint( n );

		dynamics_world_->removeConstraint( constraint );

		delete constraint;
	}

	for ( int n = dynamics_world_->getNumCollisionObjects() - 1; n >= 0; n-- )
	{
		btCollisionObject* object = dynamics_world_->getCollisionObjectArray()[ n ];
		btRigidBody* rigid_body = btRigidBody::upcast( object );

		if ( rigid_body && rigid_body->getMotionState() )
		{
			delete rigid_body->getMotionState();
		}
		
		dynamics_world_->removeCollisionObject( object );

		delete object;
	}

	for ( int n = 0; n < collision_shape_list_.size(); n++ )
	{
		delete collision_shape_list_[ n ];
	}

	for ( auto i = mesh_list_.begin(); i != mesh_list_.end(); ++i )
	{
		delete *i;
	}

	collision_shape_list_.clear();
	mesh_list_.clear();
}

/**
 * 地面用の剛体を追加する
 *
 * @param box 地面の形状
 * @return 追加された剛体
 */
btRigidBody* BulletPhysics::add_ground_rigid_body( const Vector& box )
{
	Transform transform;
	transform.set_identity();
	transform.set_position( Vector( 0, -box.y(), 0 ) );

	Transform offset;
	offset.set_identity();

	return add_box_rigid_body( transform, offset, box, 0 );
}

/**
 * 立方体形状の剛体を追加する
 * 
 * @param transform 座標変換
 * @param offset 剛体の中心点のオフセット
 * @param box 立方体の形状
 * @param mass 質量
 * @return 追加された剛体
 */
btRigidBody* BulletPhysics::add_box_rigid_body( const Transform& transform, const Transform& offset, const Vector& box, btScalar mass )
{
	btBoxShape* shape = new btBoxShape( box );
	collision_shape_list_.push_back( shape );

	return create_rigid_body( shape, transform, offset, mass );
}

/**
 * カプセル形状の剛体を追加する
 *
 * @param transform 座標変換
 * @param offset 剛体の中心点のオフセット
 * @param radius シリンダー部の上下に付加される半球の半径
 * @param height シリンダー部の高さ
 * @param mass 質量
 * @return 追加された剛体
 */
btRigidBody* BulletPhysics::add_capsule_rigid_body( const Transform& transform, const Transform& offset, float_t radius, float_t height, btScalar mass )
{
	btCapsuleShape* shape = new btCapsuleShape( radius, height );
	collision_shape_list_.push_back( shape );

	return create_rigid_body( shape, transform, offset, mass );
}

/**
 * 円柱形状の剛体を追加する
 *
 * @param transform 座標変換
 * @param offset 剛体の中心点のオフセット
 * @param radius シリンダー部の上下に付加される半球の半径
 * @param height シリンダー部の高さ
 * @param mass 質量
 * @return 追加された剛体
 */
btRigidBody* BulletPhysics::add_cylinder_rigid_body( const Transform& transform, const Transform& offset, const Vector& box, btScalar mass )
{
	btCylinderShape* shape = new btCylinderShape( box );
	collision_shape_list_.push_back( shape );

	return create_rigid_body( shape, transform, offset, mass );
}

/**
 * 剛体を生成し世界に追加する
 *
 * @param shape 形状
 * @param transform 座標変換
 * @param offset 剛体の中心点のオフセット
 * @param mass 質量
 * @return 追加された剛体
 */
btRigidBody* BulletPhysics::create_rigid_body( btCollisionShape* shape, const Transform& transform, const Transform& offset, btScalar mass )
{
	btVector3 local_inertia( 0, 0, 0 );
	shape->calculateLocalInertia( mass, local_inertia );

	btDefaultMotionState* motion_state = new btDefaultMotionState( transform * offset );
	btRigidBody::btRigidBodyConstructionInfo rigid_body_info( mass, motion_state, shape, local_inertia );
	
	btRigidBody* rigid_body = new btRigidBody( rigid_body_info );
	dynamics_world_->addRigidBody( rigid_body );

	return rigid_body;
}

/**
 * Wavefront OBJ ファイルからポリゴン形状を剛体として読み込む
 *
 */
bool BulletPhysics::load_obj( const char_t* file_name )
{
	std::ifstream in( file_name );

	if ( ! in.good() )
	{
		return false;
	}

	btTriangleMesh* triangle = new btTriangleMesh;
	std::vector< btVector3* > vs;

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

			vs.push_back( new btVector3( x, y, -z ) );
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
				triangle->addTriangle( *vs[ ns[ 0 ] ], *vs[ ns[ 1 ] ], *vs[ ns[ 2 ] ], true );
			}
		}
	}

	for ( auto i = vs.begin(); i != vs.end(); ++i )
	{
		delete *i;
	}
	
	if ( triangle->getNumTriangles() <= 0 )
	{
		delete triangle;

		COMMON_THROW_EXCEPTION_MESSAGE( string_t( "empty triangles : " ) + file_name );
	}

	mesh_list_.push_back( triangle );

	{
		btBvhTriangleMeshShape* shape = new btBvhTriangleMeshShape( triangle, true );
		collision_shape_list_.push_back( shape );

		Transform transform;
		transform.set_identity();

		// btScalar mass( 0 );
		btVector3 local_inertia( 0, 0, 0 );

		// shape->calculateLocalInertia( mass, local_inertia ); /// 静的な剛体なので慣性は不要

		btDefaultMotionState* motion_state = new btDefaultMotionState( transform );
		btRigidBody::btRigidBodyConstructionInfo rigid_body_info( 0.f, motion_state, shape, local_inertia );
	
		btRigidBody* rigid_body = new btRigidBody( rigid_body_info );
		dynamics_world_->addRigidBody( rigid_body );
	}

	return true;
}

void BulletPhysics::setConstraint()
{
	// btRigidBody* a = btRigidBody::upcast( dynamics_world_->getCollisionObjectArray()[ dynamics_world_->getNumCollisionObjects() - 2 ] );
	btRigidBody* b = btRigidBody::upcast( dynamics_world_->getCollisionObjectArray()[ dynamics_world_->getNumCollisionObjects() - 1 ] );

	// btPoint2PointConstraint* constraint = new btPoint2PointConstraint( * a, * b, Vector3( 2, 0, 0 ), Vector3( -2, 0, 0 ) );
	btPoint2PointConstraint* constraint = new btPoint2PointConstraint( * b, btVector3( 0, 5, 0 ) );

	dynamics_world_->addConstraint( constraint );
}


void BulletPhysics::update( float time_step )
{
	dynamics_world_->stepSimulation( time_step, 5, 1.f / 120.f );

	dynamics_world_->debugDrawWorld();
}

void BulletPhysics::setDebugDrawer( btIDebugDraw* debug_drawer )
{
	dynamics_world_->setDebugDrawer( debug_drawer );
}