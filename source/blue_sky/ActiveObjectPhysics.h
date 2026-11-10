#pragma once

#include <GameObject/ActiveObject.h>
#include <core/physics/Bullet/BulletPhysics.h>
#include <blue_sky/ecs/system/CollisionEventSystem.h>

namespace core::ecs { class EntityManager; }

namespace blue_sky
{

class ActiveObject;

/**
 * ActiveObject 対応 BulletPhysics
 *
 * 剛体の userPointer には物体の Entity の番号のタグ（core/physics/EntityTag.h）が入る（ActiveObject が付ける。
 * 以前は ActiveObject* を入れ、当たりの判定でキャストしてその場で on_collide_with を呼んでいた）
 */
class ActiveObjectPhysics : public BulletPhysics
{
public:
	using RigidBody = btRigidBody;

public:
	ActiveObjectPhysics();
	~ActiveObjectPhysics();

	RigidBody* add_active_object_as_box( ActiveObject* );
	RigidBody* add_active_object_as_capsule( ActiveObject* );
	RigidBody* add_active_object_as_cylinder( ActiveObject* );

	void set_active_object_info_to_rigid_body( RigidBody*, ActiveObject* );

	bool is_collision( ActiveObject*, ActiveObject* );

	/// a と接している組を集める（以前の check_collision_with。その場で on_collide_with を呼ばず、集めた組は events が振り分けて渡す）
	void collect_contacts_with( ActiveObject* a, ecs::CollisionEventSystem& events );

	/// 世界の中で接している組を全部集める（以前の check_collision_all。めり込んでいる接点 1 つにつき 1 組。順は以前と同じ）
	void collect_all_contacts( ecs::CollisionEventSystem& events );

}; // class ActiveObjectPhysics

/**
 * 自分を除き、死んだ物体・通り抜けられる物体・（足場に含めないなら）硬くない物体を通り抜けるレイ
 *
 * 当たった剛体の性質は、userPointer の Entity の番号のタグから引く（ecs::CollisionTraitsComponent）。
 * 以前は userPointer を ActiveObject* にキャストして仮想関数を呼んでいた。
 * タグの無い剛体（地面・ステージの当たりの形）には当たる。タグはあるが Entity が消えた剛体は通り抜ける
 */
class ClosestNotMe : public btCollisionWorld::ClosestRayResultCallback
{
private:
	const btRigidBody* rigid_body_;
	bool include_soft_footing_;
	const core::ecs::EntityManager* entity_manager_;

public:
	/// entity_manager : 当たった剛体の性質を引く先（物体の Entity を作った EntityManager）
	ClosestNotMe( const btVector3& s, const btVector3& d, const btRigidBody* rb, bool include_soft_footing, const core::ecs::EntityManager* entity_manager )
		: ClosestRayResultCallback( s, d )
		, rigid_body_( rb )
		, include_soft_footing_( include_soft_footing )
		, entity_manager_( entity_manager )
	{ }

	virtual	btScalar addSingleResult( btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace )
	{
		if ( rayResult.m_collisionObject == rigid_body_ )
		{
			return 1.0;
		}

		if ( ecs::CollisionEventSystem::is_vanished( entity_manager_, rayResult.m_collisionObject ) )
		{
			return 1.0;
		}

		if ( const auto* traits = ecs::CollisionEventSystem::find_traits( entity_manager_, rayResult.m_collisionObject ) )
		{
			if ( traits->is_dead )
			{
				return 1.0;
			}

			if ( traits->is_ghost )
			{
				return 1.0;
			}

			if ( ! include_soft_footing_ && ! traits->is_hard )
			{
				return 1.0;
			}
		}

		return ClosestRayResultCallback::addSingleResult( rayResult, normalInWorldSpace );
	}
};

class ClosestNotMeAndHim : public ClosestNotMe
{
private:
	const btRigidBody* him_;

public:
	ClosestNotMeAndHim( const btVector3& s, const btVector3& d, const btRigidBody* rb, const btRigidBody* him, bool include_soft_footing, const core::ecs::EntityManager* entity_manager )
		: ClosestNotMe( s, d, rb, include_soft_footing, entity_manager )
		, him_( him )
	{ }

	virtual	btScalar addSingleResult( btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
	{
		if ( rayResult.m_collisionObject == him_ )
		{
			return 1.0;
		}

		return ClosestNotMe::addSingleResult( rayResult, normalInWorldSpace );
	}
};

} // namespace blue_sky
