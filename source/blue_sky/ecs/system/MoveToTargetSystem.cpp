#include "MoveToTargetSystem.h"

#include <blue_sky/ecs/component/TargetLocationComponent.h>
#include <blue_sky/ecs/component/TargetDirectionComponent.h>
#include <blue_sky/ecs/EntityObject.h>
#include <blue_sky/DebugLog.h>

#include <core/ecs/EntityManager.h>

#include <algorithm>
#include <optional>
#include <string>

namespace blue_sky::ecs
{

void MoveToTargetSystem::add_entity_component_if_all_components_ready( const core::ecs::Entity* entity )
{
	System::add_entity_component_if_all_components_ready( entity );

	// 目標を先に付けてから窓口を付けた Entity・System を後から足したときの Entity も、目標を持っていれば回す
	add_to_target_list_if_has_target( entity );
}

void MoveToTargetSystem::add_to_target_list_if_has_target( const core::ecs::Entity* entity )
{
	const auto id = static_cast< core::ecs::EntityId >( entity->get_id() );

	if ( ! entity->get_component< TargetMoverComponent >() )
	{
		return;
	}

	if ( entity->get_component< TargetLocationComponent >() || entity->get_component< TargetDirectionComponent >() )
	{
		target_id_list_.insert( id );		// 番号は増える一方なので、たいていは末尾に足すだけ。既にあれば何もしない
	}
}

bool MoveToTargetSystem::is_watched_component_type( core::ecs::ComponentTypeId type_id ) const
{
	return type_id == core::ecs::get_component_type_id< TargetLocationComponent >() || type_id == core::ecs::get_component_type_id< TargetDirectionComponent >();
}

void MoveToTargetSystem::on_watched_component_changed( const core::ecs::Entity* entity, bool is_added )
{
	if ( is_added )
	{
		add_to_target_list_if_has_target( entity );
		return;
	}

	// 外れる部品はまだ付いている（どちらが外れるかは分からない）。目標の部品が 1 つしか無ければ、それが外れて目標が無くなる
	const int target_count = ( entity->get_component< TargetLocationComponent >() ? 1 : 0 ) + ( entity->get_component< TargetDirectionComponent >() ? 1 : 0 );

	if ( target_count <= 1 )
	{
		target_id_list_.erase( static_cast< core::ecs::EntityId >( entity->get_id() ) );
	}
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

TargetMover* MoveToTargetSystem::resolve_mover( core::ecs::EntityId id, std::optional< EntityObject >& storage ) const
{
	auto* mover_component = find_component< TargetMoverComponent >( id );

	if ( ! mover_component )
	{
		return nullptr;
	}

	if ( mover_component->mover )
	{
		return mover_component->mover;
	}

	// ActiveObject を持たない物体は、部品を直接動かす（位置の正が無ければ動かせない）
	storage.emplace( get_entity_manager()->find( id ) );

	return storage->is_valid() ? & * storage : nullptr;
}

bool MoveToTargetSystem::is_target_location_reached( float_t distance, float_t speed, float_t delta_time )
{
	return distance < ( std::max )( 0.1f, speed * delta_time );
}

void MoveToTargetSystem::update_targets( float_t delta_time )
{
	// 目標を持つものだけを回す。知らせた先で物体が加わる・消える・目標が付け外しされることがあるので、
	// 番号の一覧を写してから回し、部品は 1 つずつ引き直す
	if ( target_id_list_.size() == 0 )
	{
		return;
	}

	const std::vector< core::ecs::EntityId > ids = get_target_ids();

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

	if ( ! target )
	{
		return;
	}

	std::optional< EntityObject > entity_object;
	TargetMover* mover = resolve_mover( id, entity_object );

	if ( ! mover )
	{
		return;
	}

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

	if ( ! direction )
	{
		return;
	}

	std::optional< EntityObject > entity_object;
	TargetMover* mover = resolve_mover( id, entity_object );

	if ( ! mover )
	{
		return;
	}

	if ( ! direction->is_entity_target )
	{
		mover->chase_target_direction_degree( direction->degree, direction->speed );
		return;
	}

	std::optional< EntityObject > target_entity_object;
	TargetMover* target_mover = resolve_mover( direction->target_entity, target_entity_object );

	if ( ! target_mover )
	{
		// 向く先の物体が消えた。目標を捨てる（以前は消えた物体を指したままになり得た）
		get_entity_manager()->remove_component< TargetDirectionComponent >( get_entity_manager()->find( id ) );
		return;
	}

	mover->chase_target_direction_to( target_mover->get_target_mover_location(), direction->speed );
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
	for ( const auto id : std::vector< core::ecs::EntityId >( get_target_ids() ) )
	{
		if ( core::ecs::Entity* entity = get_entity_manager()->find( id ) )
		{
			entity->remove_component< TargetLocationComponent >();
			entity->remove_component< TargetDirectionComponent >();
		}
	}
}

} // namespace blue_sky::ecs
