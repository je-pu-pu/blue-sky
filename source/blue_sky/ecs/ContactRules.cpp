#include "ContactRules.h"

#include <blue_sky/GameObjectRef.h>
#include <blue_sky/ecs/EntityObject.h>
#include <blue_sky/ecs/ObjectBody.h>
#include <blue_sky/ecs/component/StaticObjectComponent.h>
#include <blue_sky/ecs/component/PickupComponent.h>
#include <blue_sky/ecs/component/ContactCacheComponent.h>
#include <blue_sky/ecs/component/SwitchComponent.h>
#include <blue_sky/ecs/component/BalloonHoldComponent.h>
#include <blue_sky/ecs/component/RobotBehaviorComponent.h>
#include <blue_sky/ecs/component/PlayerBehaviorComponent.h>
#include <blue_sky/ecs/system/SwitchSystem.h>
#include <blue_sky/ecs/system/BalloonHoldSystem.h>
#include <blue_sky/ecs/system/RobotBehaviorSystem.h>
#include <blue_sky/ecs/system/PlayerBehaviorSystem.h>

#include <core/ecs/EntityManager.h>

namespace blue_sky::ecs::contact_rules
{

void on_collide_with_player( core::ecs::Entity& player, core::ecs::Entity& entity, core::sound::SoundManager* sound_manager )
{
	auto* behavior = player.get_entity_manager()->get_system< PlayerBehaviorSystem >();

	if ( ! behavior )
	{
		return;
	}

	const auto player_id = static_cast< core::ecs::EntityId >( player.get_id() );

	// 動かない物（以前の StaticObject）: 1 組につき 1 回、プレイヤーに知らせる（ロケットで飛んでいれば止まる）
	if ( entity.get_component< StaticObjectComponent >() )
	{
		behavior->on_touch_static_object( player_id );
	}

	// 拾う物（以前の Medal・Rocket・Umbrella）: プレイヤーが拾ったら、この物体を死なせる（ロケットは当たりの応答も消す）
	if ( const auto* pickup = entity.get_component< PickupComponent >() )
	{
		if ( behavior->on_touch_pickup( player_id, pickup->kind ) )
		{
			EntityObject picked( & entity );
			picked.kill();

			if ( pickup->disable_contact_on_pickup && picked.get_rigid_body() )
			{
				object_body::set_no_contact_response( * picked.get_rigid_body(), true );
			}
		}
	}

	// 石（以前の Stone）: プレイヤーが拾ったら石を死なせる（プレイヤーは窓口を持っておいて投げる）
	if ( entity.get_component< ThrowableComponent >() )
	{
		if ( behavior->on_touch_stone( player_id, GameObjectRef( entity.get_entity_manager(), nullptr, static_cast< core::ecs::EntityId >( entity.get_id() ) ) ) )
		{
			EntityObject( & entity ).kill();
		}
	}

	// スイッチ（以前の BaseSwitch）: プレイヤーの速さで入・切が変わる
	if ( entity.get_component< SwitchComponent >() )
	{
		if ( auto* system = entity.get_entity_manager()->get_system< SwitchSystem >() )
		{
			system->on_touch( static_cast< core::ecs::EntityId >( entity.get_id() ), player_id, EntityObject( & player ).get_velocity() );
		}
	}

	// 風船（以前の Balloon）: つかまれていない風船だけ知らせ、プレイヤーがつかんだら風船をつかませる
	if ( const auto* balloon = entity.get_component< BalloonHoldComponent >(); balloon && ! balloon->is_held() )
	{
		const auto balloon_id = static_cast< core::ecs::EntityId >( entity.get_id() );

		if ( behavior->on_touch_balloon( player_id, balloon_id ) )
		{
			if ( auto* system = entity.get_entity_manager()->get_system< BalloonHoldSystem >() )
			{
				system->grab( balloon_id, player_id );
			}
		}
	}

	// 敵（以前の Robot）: プレイヤーに「追っているか」を知らせ（追う敵に当たると傷つく）、敵は立つ・回る間なら止まる
	if ( entity.get_component< RobotBehaviorComponent >() )
	{
		const auto robot_id = static_cast< core::ecs::EntityId >( entity.get_id() );
		auto* system = entity.get_entity_manager()->get_system< RobotBehaviorSystem >();
		const bool is_chasing = system && system->get_mode( robot_id ) == RobotBehaviorComponent::Mode::CHASE;

		if ( behavior->on_touch_robot( player_id, is_chasing ) && system )
		{
			system->on_touched_by_target( robot_id );
		}
	}

	// 梯子（以前の Ladder）: プレイヤーが梯子につかまり、梯子は最近（0.5 秒以内）触れていなければ音を鳴らして覚える
	if ( entity.get_component< LadderComponent >() )
	{
		if ( behavior->on_touch_ladder( player_id ) )
		{
			if ( auto* cache = entity.get_component< ContactCacheComponent >() )
			{
				if ( ! cache->cache.is_cached( player_id ) )
				{
					EntityObject( & entity ).play_sound( sound_manager, "ladder-contact" );
				}

				cache->cache.cache( player_id, ContactCacheComponent::LifeTime );
			}
		}
	}
}

void on_collide( core::ecs::Entity& a, core::ecs::Entity& b, core::sound::SoundManager* sound_manager )
{
	// プレイヤーと物体 : 物体の部品の組み合わせで、プレイヤーの System に知らせる（1 組につき 1 回。以前はプレイヤーが ActiveObject で、on_collide_with_entity_object だった）
	if ( a.get_component< PlayerBehaviorComponent >() )
	{
		on_collide_with_player( a, b, sound_manager );
		return;
	}

	if ( b.get_component< PlayerBehaviorComponent >() )
	{
		on_collide_with_player( b, a, sound_manager );
		return;
	}

	// 石と風船 : 以前は Stone::on_collide_with( GameObject* ) が風船の on_collide_with( Stone* )（今の on_collide_with_stone）を呼び、
	// 風船は割れて音を鳴らした（石は拾わない）。1 組につき 1 回
	auto burst_by_stone = [] ( core::ecs::Entity& balloon, core::ecs::Entity& stone )
	{
		if ( ! balloon.get_component< BalloonHoldComponent >() || ! stone.get_component< ThrowableComponent >() )
		{
			return false;
		}

		if ( auto* system = balloon.get_entity_manager()->get_system< BalloonHoldSystem >() )
		{
			system->burst( static_cast< core::ecs::EntityId >( balloon.get_id() ) );
		}

		return true;
	};

	if ( burst_by_stone( a, b ) || burst_by_stone( b, a ) )
	{
		return;
	}

	// 石と敵 : 以前は Stone::on_collide_with( GameObject* ) が敵の on_collide_with_stone を呼び、
	// 敵は立つ・回る間に、こちらへ飛んでくる石なら止まった（石は拾わない）。1 組につき 1 回
	auto stop_by_stone = [] ( core::ecs::Entity& robot, core::ecs::Entity& stone )
	{
		if ( ! robot.get_component< RobotBehaviorComponent >() || ! stone.get_component< ThrowableComponent >() )
		{
			return false;
		}

		if ( auto* system = robot.get_entity_manager()->get_system< RobotBehaviorSystem >() )
		{
			const EntityObject s( & stone );
			const EntityObject r( & robot );

			system->on_hit_by_stone( static_cast< core::ecs::EntityId >( robot.get_id() ), object_body::is_moving_to( s.get_location(), s.get_velocity(), r.get_location() ) );
		}

		return true;
	};

	if ( stop_by_stone( a, b ) || stop_by_stone( b, a ) )
	{
		return;
	}

	// 以前は Stone::on_collide_with( GameObject* ) がスイッチの on_collide_with( Stone* ) を呼び（1 組につき 1 回）、
	// スイッチ → 石（on_collide_with( BaseSwitch* )）は何もしなかった
	auto press_by_stone = [] ( core::ecs::Entity& s, core::ecs::Entity& stone )
	{
		const auto* switch_component = s.get_component< SwitchComponent >();

		if ( ! switch_component || ! switch_component->is_switched_by_stone || ! stone.get_component< ThrowableComponent >() )
		{
			return false;
		}

		if ( auto* system = s.get_entity_manager()->get_system< SwitchSystem >() )
		{
			system->on_touch( static_cast< core::ecs::EntityId >( s.get_id() ), static_cast< core::ecs::EntityId >( stone.get_id() ), EntityObject( & stone ).get_velocity() );
		}

		return true;
	};

	if ( ! press_by_stone( a, b ) )
	{
		press_by_stone( b, a );
	}
}

} // namespace blue_sky::ecs::contact_rules
