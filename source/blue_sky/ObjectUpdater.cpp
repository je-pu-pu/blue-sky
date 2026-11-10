#include "ObjectUpdater.h"
#include <core/physics/PhysicsManager.h>

#include <GameObject/GameObjectContext.h>

#include <blue_sky/ecs/ContactRules.h>
#include <blue_sky/ecs/EntityObject.h>
#include <blue_sky/ecs/system/ObjectRenderDataSystem.h>
#include <blue_sky/ecs/system/MoveToTargetSystem.h>
#include <blue_sky/ecs/system/PhysicsSyncSystem.h>
#include <blue_sky/ecs/system/TranslationSystem.h>
#include <blue_sky/ecs/system/SpinSystem.h>
#include <blue_sky/ecs/system/ContactCacheSystem.h>
#include <blue_sky/ecs/system/BalloonHoldSystem.h>
#include <blue_sky/ecs/system/GirlSystem.h>
#include <blue_sky/ecs/system/RobotBehaviorSystem.h>
#include <blue_sky/ecs/system/PlayerBehaviorSystem.h>

#include <core/ecs/EntityManager.h>
#include <blue_sky/FrameStageTimer.h>

namespace blue_sky
{

ObjectUpdater::ObjectUpdater( const core::physics::PhysicsManager* physics, const GameObjectContext* object_context, core::ecs::EntityManager* entity_manager )
	: physics_( physics )
	, object_context_( object_context )
	, entity_manager_( entity_manager )
{

}

float_t ObjectUpdater::get_delta_time() const
{
	return object_context_ && object_context_->delta_time ? object_context_->delta_time() : 0.f;
}

float_t ObjectUpdater::get_scene_elapsed_time() const
{
	return object_context_ && object_context_->scene_elapsed_time ? object_context_->scene_elapsed_time() : 0.f;
}

void ObjectUpdater::mark_stage( const char* name ) const
{
	if ( stage_timer_ )
	{
		stage_timer_->mark( name );
	}
}

void ObjectUpdater::update()
{
	const float_t delta_time = get_delta_time();
	const float_t scene_elapsed_time = get_scene_elapsed_time();

	// プレイヤー（以前は ActiveObject の一覧の先頭の Player::update。ほかの物体より先に）
	if ( auto* system = entity_manager_->get_system< ecs::PlayerBehaviorSystem >() )
	{
		system->update_players( delta_time, scene_elapsed_time );
	}

	mark_stage( "System : プレイヤー" );

	// 風船（以前は Balloon::update。アニメーションより前に）
	if ( auto* system = entity_manager_->get_system< ecs::BalloonHoldSystem >() )
	{
		system->update_balloons( scene_elapsed_time );
	}

	mark_stage( "System : 風船" );

	// 少女（以前は Girl::update）
	if ( auto* system = entity_manager_->get_system< ecs::GirlSystem >() )
	{
		system->update_girls( scene_elapsed_time );
	}

	mark_stage( "System : 少女" );

	// 敵（以前は Robot::update。アニメーションを変えるので、アニメーションを進める前に）
	if ( auto* system = entity_manager_->get_system< ecs::RobotBehaviorSystem >() )
	{
		system->update_robots( delta_time, scene_elapsed_time );
	}

	mark_stage( "System : 敵" );

	// アニメーションを進める（物体のアニメーションは互いに独立なので、まとめて後で進めても同じ）
	if ( auto* system = entity_manager_->get_system< ecs::ObjectRenderDataSystem >() )
	{
		system->update_animations();
	}

	mark_stage( "System : アニメーション" );

	// 動く床（以前は TranslationObject::update）
	if ( auto* system = entity_manager_->get_system< ecs::TranslationSystem >() )
	{
		system->update_translations( delta_time );
	}

	mark_stage( "System : 動く床" );

	// 最近触れた物体の覚えを減らす（以前は Ladder::update など）
	if ( auto* system = entity_manager_->get_system< ecs::ContactCacheSystem >() )
	{
		system->update_caches( delta_time );
	}

	mark_stage( "System : 触れた物の覚え" );

	// 回る物（以前は Medal::update など）
	if ( auto* system = entity_manager_->get_system< ecs::SpinSystem >() )
	{
		system->update_spins( delta_time );
	}

	mark_stage( "System : 回る物" );

	// 目標地点・向きへ動かす（目標を持つ物体を Entity の番号の順に回す）
	if ( auto* system = entity_manager_->get_system< ecs::MoveToTargetSystem >() )
	{
		system->update_targets( delta_time );
	}

	mark_stage( "System : 目標地点へ動かす" );
}

void ObjectUpdater::update_balloon( core::ecs::EntityId id )
{
	if ( auto* system = entity_manager_->get_system< ecs::BalloonHoldSystem >() )
	{
		system->update_balloon( id, get_scene_elapsed_time() );
	}
}

void ObjectUpdater::update_transforms()
{
	if ( auto* system = entity_manager_->get_system< ecs::PhysicsSyncSystem >() )
	{
		system->update_transforms();
	}
}

void ObjectUpdater::check_collision_with( core::ecs::EntityId id )
{
	auto* events = entity_manager_->get_system< ecs::CollisionEventSystem >();
	core::ecs::Entity* entity = entity_manager_->find( id );
	const core::physics::RigidBody* body = entity ? ecs::EntityObject( entity ).get_rigid_body() : nullptr;

	if ( ! events || ! body || ! physics_ )
	{
		return;
	}

	// 接している組を集める（以前の ActiveObjectPhysics::collect_contacts_with。その場で振り分けず、集めた組は events が渡す）
	physics_->contact_test( * body, [ events ] ( std::optional< core::ecs::EntityId > a, std::optional< core::ecs::EntityId > b ) { events->add_contact( a, b ); } );
	events->dispatch( * this );
}

void ObjectUpdater::check_collision_all()
{
	auto* events = entity_manager_->get_system< ecs::CollisionEventSystem >();

	if ( ! events || ! physics_ )
	{
		return;
	}

	// めり込んでいる接点 1 つにつき 1 組を集める（以前の ActiveObjectPhysics::collect_all_contacts。順も同じ）
	physics_->for_each_penetrating_contact( [ events ] ( std::optional< core::ecs::EntityId > a, std::optional< core::ecs::EntityId > b ) { events->add_contact( a, b ); } );
	events->dispatch( * this );
}

/// 物体が地面に当たった : 何も起こさない（以前は ActiveObject の on_collide_with_ground で、受け取る種類は無かった）
void ObjectUpdater::on_collide_with_ground( core::ecs::EntityId )
{

}

void ObjectUpdater::on_collide( core::ecs::EntityId a, core::ecs::EntityId b )
{
	core::ecs::Entity* e0 = entity_manager_->find( a );
	core::ecs::Entity* e1 = entity_manager_->find( b );

	if ( e0 && e1 )
	{
		ecs::contact_rules::on_collide( * e0, * e1, object_context_ ? object_context_->sound_manager : nullptr );
	}
}

} // namespace blue_sky
