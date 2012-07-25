#include "BulletPhysics.h"

#ifdef _DEBUG
#pragma comment( lib, "BulletCollision_debug.lib" )
#pragma comment( lib, "BulletDynamics_debug.lib" )
#pragma comment( lib, "LinearMath_debug.lib" )
#else
#pragma comment( lib, "BulletCollision.lib" )
#pragma comment( lib, "BulletDynamics.lib" )
#pragma comment( lib, "LinearMath.lib" )
#endif

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

	// create_ground_shape()
	btCollisionShape* ground = new btBoxShape( btVector3( btScalar( 1000 ), btScalar( 1 ), btScalar( 1000 ) ) );
	btAlignedObjectArray<btCollisionShape*> collision_shape_list_;

	collision_shape_list_.push_back( ground );

	// create_ground_rigid_body()
	{
		btTransform ground_transform;
		ground_transform.setIdentity();
		ground_transform.setOrigin( btVector3( 0, -1, 0 ) );

		btScalar mass( 0 );
		btVector3 local_inertia( 0, 0, 0 );

		ground->calculateLocalInertia( mass, local_inertia );

		btDefaultMotionState* motion_state = new btDefaultMotionState( ground_transform );
		btRigidBody::btRigidBodyConstructionInfo rigid_body_info( mass, motion_state, ground, local_inertia );
	
		btRigidBody* rigid_body = new btRigidBody( rigid_body_info );
		dynamics_world_->addRigidBody( rigid_body );
	}
}

BulletPhysics::~BulletPhysics()
{
	for ( int n = dynamics_world_->getNumConstraints() - 1; n >= 0; n-- )
	{
		delete dynamics_world_->getConstraint( n );
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
		btCollisionShape* shape = collision_shape_list_[ n ];
		collision_shape_list_[ n ] = 0;
		delete shape;
	}

	delete dynamics_world_;
	delete constraint_solver_;
	delete broadphase_interface_;
	delete collision_dispatcher_;
	delete collision_configuration_;
}

btRigidBody* BulletPhysics::add_box_rigid_body( const Transform& transform, const Transform& offset, const btVector3& box, bool is_static )
{
	// 
	// btCompoundShape* compound_shape = new btCompoundShape();
	// collision_shape_list_.push_back( compound_shape );

	// create_box_shape()
	btBoxShape* shape = new btBoxShape( box );
	collision_shape_list_.push_back( shape );

	// compound_shape->addChildShape( offset, shape );

	// create_box_rigid_body()
	{
		btScalar mass( is_static ? 0.f : 100.f );
		btVector3 local_inertia( 0, 0, 0 );

		shape->calculateLocalInertia( mass, local_inertia );
		// compound_shape->calculateLocalInertia( mass, local_inertia );

		btDefaultMotionState* motion_state = new btDefaultMotionState( transform * offset );
		btRigidBody::btRigidBodyConstructionInfo rigid_body_info( mass, motion_state, shape, local_inertia );
	
		btRigidBody* rigid_body = new btRigidBody( rigid_body_info );
		dynamics_world_->addRigidBody( rigid_body );

		return rigid_body;
	}	
}

void BulletPhysics::setConstraint()
{
	btRigidBody* a = btRigidBody::upcast( dynamics_world_->getCollisionObjectArray()[ dynamics_world_->getNumCollisionObjects() - 2 ] );
	btRigidBody* b = btRigidBody::upcast( dynamics_world_->getCollisionObjectArray()[ dynamics_world_->getNumCollisionObjects() - 1 ] );

	// btPoint2PointConstraint* constraint = new btPoint2PointConstraint( * a, * b, Vector3( 2, 0, 0 ), Vector3( -2, 0, 0 ) );
	btPoint2PointConstraint* constraint = new btPoint2PointConstraint( * b, Vector3( 0, 5, 0 ) );

	dynamics_world_->addConstraint( constraint );
}


void BulletPhysics::update( float time_step )
{
	dynamics_world_->stepSimulation( time_step );

	dynamics_world_->debugDrawWorld();
}

void BulletPhysics::setDebugDrawer( btIDebugDraw* debug_drawer )
{
	dynamics_world_->setDebugDrawer( debug_drawer );
}