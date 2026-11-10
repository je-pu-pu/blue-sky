#include "MoveToTargetSystem.h"

#include <blue_sky/ecs/component/TargetLocationComponent.h>
#include <blue_sky/ecs/component/TargetDirectionComponent.h>
#include <blue_sky/DebugLog.h>

#include <core/ecs/EntityManager.h>

#include <algorithm>
#include <string>

namespace blue_sky::ecs
{

void MoveToTargetSystem::add_entity_component_if_all_components_ready( const core::ecs::Entity* entity )
{
	System::add_entity_component_if_all_components_ready( entity );

	is_entity_id_list_dirty_ = true;
}

const std::vector< core::ecs::EntityId >& MoveToTargetSystem::get_entity_ids() const
{
	if ( is_entity_id_list_dirty_ )
	{
		entity_id_list_.clear();
		entity_id_list_.reserve( get_component_list().size() );

		for ( const auto& i : get_component_list() )
		{
			entity_id_list_.push_back( i.first );
		}

		std::sort( entity_id_list_.begin(), entity_id_list_.end() );

		is_entity_id_list_dirty_ = false;
	}

	return entity_id_list_;
}

template< typename ComponentType >
ComponentType* MoveToTargetSystem::find_component( core::ecs::EntityId id ) const
{
	if ( ! get_entity_manager() )
	{
		return nullptr;
	}

	core::ecs::Entity* entity = get_entity_manager()->find( id );

	return entity ? entity->get_component< ComponentType >() : nullptr;
}

bool MoveToTargetSystem::is_target_location_reached( float_t distance, float_t speed, float_t delta_time )
{
	return distance < ( std::max )( 0.1f, speed * delta_time );
}

void MoveToTargetSystem::update_targets( float_t delta_time )
{
	// 知らせた先で物体が加わる・消えることがあるので、番号の一覧を写してから回し、部品は 1 つずつ引き直す
	const std::vector< core::ecs::EntityId > ids = get_entity_ids();

	// 以前と同じ順に、目標地点 → 向き の順で回す
	for ( const auto id : ids )
	{
		update_target_location( id, delta_time );
	}

	for ( const auto id : ids )
	{
		update_target_direction( id );
	}
}

void MoveToTargetSystem::update_target_location( core::ecs::EntityId id, float_t delta_time )
{
	auto* target = find_component< TargetLocationComponent >( id );
	auto* mover_component = find_component< TargetMoverComponent >( id );

	if ( ! target || ! mover_component || ! mover_component->mover )
	{
		return;
	}

	TargetMover* mover = mover_component->mover;
	const core::Vector location = target->location;
	const float_t speed = target->speed;

	if ( ! is_target_location_reached( ( location - mover->get_target_mover_location() ).length(), speed, delta_time ) )
	{
		mover->move_toward_target_location( location, speed );
		return;
	}

	// 目標の位置に合わせて止める
	// （以前は 0.1 未満に入ったときだけ着いたことにしていて、速いとき・フレームが長いときは目標の周りを行ったり来たりして着かないことがあった。
	// 着いた後も最後の速度が残り、着いたときの処理を持たない物体（壁・プレイヤー）は動き続けていた）
	mover->stop_at_target_location( location );

	// 知らせる前に外す（知らせた先で新しい目標を設定しても消えないように。以前は知らせた後に消していた）
	get_entity_manager()->remove_component< TargetLocationComponent >( get_entity_manager()->find( id ) );

	mover->notify_arrive_at_target_location();

	debug_log( "[Object] 目標地点に着きました ( " + std::to_string( location.x() ) + ", " + std::to_string( location.y() ) + ", " + std::to_string( location.z() ) + " )" );
}

void MoveToTargetSystem::update_target_direction( core::ecs::EntityId id )
{
	auto* direction = find_component< TargetDirectionComponent >( id );
	auto* mover_component = find_component< TargetMoverComponent >( id );

	if ( ! direction || ! mover_component || ! mover_component->mover )
	{
		return;
	}

	if ( ! direction->is_entity_target )
	{
		mover_component->mover->chase_target_direction_degree( direction->degree, direction->speed );
		return;
	}

	auto* target_mover_component = find_component< TargetMoverComponent >( direction->target_entity );

	if ( ! target_mover_component || ! target_mover_component->mover )
	{
		// 向く先の物体が消えた。目標を捨てる（以前は消えた物体を指したままになり得た）
		get_entity_manager()->remove_component< TargetDirectionComponent >( get_entity_manager()->find( id ) );
		return;
	}

	mover_component->mover->chase_target_direction_to( target_mover_component->mover->get_target_mover_location(), direction->speed );
}

bool MoveToTargetSystem::set_target_location( core::ecs::EntityId id, const core::Vector& location, float_t speed )
{
	if ( ! find_component< TargetMoverComponent >( id ) )
	{
		return false;
	}

	core::ecs::Entity* entity = get_entity_manager()->find( id );
	auto* target = entity->get_component< TargetLocationComponent >();

	if ( ! target )
	{
		target = entity->add_component< TargetLocationComponent >();
	}

	target->location = location;
	target->speed = speed;

	return true;
}

bool MoveToTargetSystem::set_target_direction( core::ecs::EntityId id, float_t degree, float_t speed )
{
	if ( ! find_component< TargetMoverComponent >( id ) )
	{
		return false;
	}

	core::ecs::Entity* entity = get_entity_manager()->find( id );
	auto* direction = entity->get_component< TargetDirectionComponent >();

	if ( ! direction )
	{
		direction = entity->add_component< TargetDirectionComponent >();
	}

	direction->is_entity_target = false;
	direction->degree = degree;
	direction->speed = speed;

	return true;
}

bool MoveToTargetSystem::set_target_direction_entity( core::ecs::EntityId id, core::ecs::EntityId target_id, float_t speed )
{
	if ( ! find_component< TargetMoverComponent >( id ) )
	{
		return false;
	}

	core::ecs::Entity* entity = get_entity_manager()->find( id );
	auto* direction = entity->get_component< TargetDirectionComponent >();

	if ( ! direction )
	{
		direction = entity->add_component< TargetDirectionComponent >();
	}

	direction->is_entity_target = true;
	direction->target_entity = target_id;
	direction->speed = speed;

	return true;
}

void MoveToTargetSystem::clear_targets()
{
	for ( const auto id : std::vector< core::ecs::EntityId >( get_entity_ids() ) )
	{
		if ( core::ecs::Entity* entity = get_entity_manager()->find( id ) )
		{
			entity->remove_component< TargetLocationComponent >();
			entity->remove_component< TargetDirectionComponent >();
		}
	}
}

} // namespace blue_sky::ecs
