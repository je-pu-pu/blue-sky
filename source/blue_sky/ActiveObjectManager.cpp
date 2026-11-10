#include "ActiveObjectManager.h"
#include "ActiveObjectPhysics.h"
#include "DebugLog.h"

#include <blue_sky/GameObjectRef.h>

#include <GameObject/ActiveObject.h>
#include <GameObject/GameObjectContext.h>

#include <algorithm>

#include <blue_sky/GameMain.h>
#include <blue_sky/graphics/GraphicsManager.h>

#include <blue_sky/ecs/system/ObjectRenderDataSystem.h>
#include <blue_sky/ecs/system/MoveToTargetSystem.h>
#include <blue_sky/ecs/system/CollisionEventSystem.h>
#include <blue_sky/ecs/system/PhysicsSyncSystem.h>
#include <blue_sky/ecs/system/TranslationSystem.h>
#include <blue_sky/ecs/component/ActiveObjectLinkComponent.h>
#include <blue_sky/ecs/component/StaticObjectComponent.h>
#include <blue_sky/ecs/component/PickupComponent.h>
#include <blue_sky/ecs/system/SpinSystem.h>
#include <blue_sky/ecs/system/ContactCacheSystem.h>
#include <blue_sky/ecs/component/ContactCacheComponent.h>
#include <blue_sky/ecs/component/SwitchComponent.h>
#include <blue_sky/ecs/system/SwitchSystem.h>
#include <blue_sky/ecs/system/BalloonHoldSystem.h>
#include <blue_sky/ecs/system/GirlSystem.h>
#include <blue_sky/ecs/system/RobotBehaviorSystem.h>
#include <blue_sky/ecs/component/RobotBehaviorComponent.h>
#include <blue_sky/ecs/component/BalloonHoldComponent.h>
#include <blue_sky/ecs/EntityObject.h>
#include <blue_sky/ecs/ObjectBody.h>

#include <core/ecs/EntityManager.h>

namespace blue_sky
{

ActiveObjectManager::ActiveObjectManager( ActiveObjectPhysics* physics, graphics::GraphicsManager* graphics_manager, const GameObjectContext* object_context, core::ecs::EntityManager* entity_manager )
	: physics_( physics )
	, graphics_manager_( graphics_manager )
	, object_context_( object_context )
	, entity_manager_( entity_manager )
{

	// "static"（動かない物）・"medal"・"umbrella"・"rocket"・"goal"・"ladder"・"stone"・"switch"（と空間スイッチ）は ECS だけの物体になり、ObjectFactory が作る（段階 4）。
	// "balloon"・"girl"・"robot" も ECS だけの物体（段階 5）。名前から作る ActiveObject の種類は今は無い（プレイヤーはシーンが作る）

}

ActiveObjectManager::~ActiveObjectManager()
{
	clear();
}

void ActiveObjectManager::clear()
{
	for ( ActiveObjectList::iterator i = active_object_list_.begin(); i != active_object_list_.end(); ++i )
	{
		delete *i;
	}

	// 目標（部品）は、物体を壊すときに Entity ごと消える。名前の表は ObjectFactory が持つ
	active_object_list_.clear();
}

void ActiveObjectManager::restart()
{
	for ( auto i = active_object_list_.begin(); i != active_object_list_.end(); ++i )
	{
		( *i )->restart();
	}

	if ( auto* system = entity_manager_->get_system< ecs::MoveToTargetSystem >() )
	{
		system->clear_targets();
	}
}

void ActiveObjectManager::add_active_object( ActiveObject* active_object )
{
	active_object->set_context( object_context_ );

	// 描画のデータを更新する System（シーンの切り替えで EntityManager が片付けるので、無ければ足す。あれば何もしない）
	entity_manager_->add_system< ecs::ObjectRenderDataSystem >( 0, graphics_manager_ );

	// 目標地点・向きへ動かす System（同上）
	entity_manager_->add_system< ecs::MoveToTargetSystem >( 0 );

	// 当たった組を集めて振り分ける System（同上）
	entity_manager_->add_system< ecs::CollisionEventSystem >( 0 );

	// 剛体の位置を位置の正（TransformComponent）へ写す System（同上）
	entity_manager_->add_system< ecs::PhysicsSyncSystem >( 0 );

	// Entity を作り、描画の部品と定数バッファを用意する（以前は ActiveObject が定数バッファ・Model* を自分で持っていた）
	active_object->attach_entity( entity_manager_, graphics_manager_ );

	active_object_list_.push_back( active_object );
}

/**
 * 指定した名前のオブジェクトを生成する
 *
 * @param name オブジェクト名
 * @return 生成された ActiveObject ( または失敗時に nullptr を返す )
 */
ActiveObject* ActiveObjectManager::create_object( const string_t& name )
{
	auto i = object_creator_map_.find( name );

	if ( i == object_creator_map_.end() )
	{
		return nullptr;
	}

	ActiveObject* active_object = i->second();
	active_object->set_context( object_context_ );		// 作った直後から使えるように（加える前に初期化で使うものがある）

	add_active_object( active_object );

	return active_object;
}

ActiveObject* ActiveObjectManager::clone_object( const ActiveObject* o )
{
	ActiveObject* o2 = o->clone();

	add_active_object( o2 );

	return o2;
}

void ActiveObjectManager::update()
{
	// 加えた順に更新する。更新の中で物体が加えられても一覧の参照が壊れないように添字で回し、
	// 加えられた物体は次のフレームから更新する（始めの数までで止める）
	const size_t object_count = active_object_list_.size();

	for ( size_t i = 0; i < object_count; ++i )
	{
		active_object_list_[ i ]->update();
	}

	const float_t scene_elapsed_time = object_context_ && object_context_->scene_elapsed_time ? object_context_->scene_elapsed_time() : 0.f;

	// 風船（ECS だけの物体。以前は Balloon::update が物体の update の中で動かしていた。アニメーションより前に）
	if ( auto* system = entity_manager_->get_system< ecs::BalloonHoldSystem >() )
	{
		system->update_balloons( scene_elapsed_time );
	}

	// 少女（ECS だけの物体。以前は Girl::update）
	if ( auto* system = entity_manager_->get_system< ecs::GirlSystem >() )
	{
		system->update_girls( scene_elapsed_time );
	}

	// 敵（ECS だけの物体。以前は Robot::update。アニメーションを変えるので、アニメーションを進める前に）
	if ( auto* system = entity_manager_->get_system< ecs::RobotBehaviorSystem >() )
	{
		system->update_robots( object_context_ && object_context_->delta_time ? object_context_->delta_time() : 0.f, scene_elapsed_time );
	}

	// アニメーションを進める（以前は物体ごとに update の直後に進めていた。物体のアニメーションは互いに独立なので、まとめて後で進めても同じ）
	if ( auto* system = entity_manager_->get_system< ecs::ObjectRenderDataSystem >() )
	{
		system->update_animations();
	}

	const float_t delta_time = object_context_ && object_context_->delta_time ? object_context_->delta_time() : 0.f;

	// 動く床（ECS だけの物体。以前は TranslationObject::update が物体の update の中で動かしていた）
	if ( auto* system = entity_manager_->get_system< ecs::TranslationSystem >() )
	{
		system->update_translations( delta_time );
	}

	// 最近触れた物体の覚えを減らす（ECS だけの物体。以前は Ladder::update が物体の update の中で減らしていた）
	if ( auto* system = entity_manager_->get_system< ecs::ContactCacheSystem >() )
	{
		system->update_caches( delta_time );
	}

	// 回る物（ECS だけの物体。以前は Medal::update）
	if ( auto* system = entity_manager_->get_system< ecs::SpinSystem >() )
	{
		system->update_spins( delta_time );
	}

	// 目標地点・向きへ動かす（以前はここで 3 つの unordered_map を回していた。今は目標を持つ物体を Entity の番号の順に回す）
	if ( auto* system = entity_manager_->get_system< ecs::MoveToTargetSystem >() )
	{
		system->update_targets( delta_time );
	}
}

void ActiveObjectManager::update_balloon( core::ecs::EntityId id )
{
	const float_t scene_elapsed_time = object_context_ && object_context_->scene_elapsed_time ? object_context_->scene_elapsed_time() : 0.f;

	if ( auto* system = entity_manager_->get_system< ecs::BalloonHoldSystem >() )
	{
		system->update_balloon( id, scene_elapsed_time );
	}
}

void ActiveObjectManager::update_transforms()
{
	if ( auto* system = entity_manager_->get_system< ecs::PhysicsSyncSystem >() )
	{
		system->update_transforms();
	}
}

void ActiveObjectManager::check_collision_with( ActiveObject* a )
{
	auto* events = entity_manager_->get_system< ecs::CollisionEventSystem >();

	if ( ! events || ! a || ! a->get_rigid_body() )
	{
		return;
	}

	physics_->collect_contacts_with( a, * events );
	events->dispatch( * this );
}

void ActiveObjectManager::check_collision_all()
{
	auto* events = entity_manager_->get_system< ecs::CollisionEventSystem >();

	if ( ! events )
	{
		return;
	}

	physics_->collect_all_contacts( * events );
	events->dispatch( * this );
}

ActiveObject* ActiveObjectManager::find_active_object( core::ecs::EntityId id ) const
{
	core::ecs::Entity* entity = entity_manager_->find( id );
	const auto* link = entity ? entity->get_component< ecs::ActiveObjectLinkComponent >() : nullptr;

	return link ? link->object : nullptr;
}

/// 以前の ActiveObjectContactResultCallback::on_collide の「片方が地面」の所
void ActiveObjectManager::on_collide_with_ground( core::ecs::EntityId id )
{
	if ( ActiveObject* a = find_active_object( id ) )
	{
		a->on_collide_with_ground();
	}
}

/**
 * 以前の ActiveObjectContactResultCallback::on_collide の「両方が物体」の所
 *
 * - 両方とも ActiveObject : 今の二重ディスパッチ（同じ順に両方へ知らせる）
 * - 片方だけ ActiveObject : もう片方（ECS だけの物体）の部品の組み合わせで決める（on_collide_with_entity_object）
 * - 両方とも ECS だけの物体 : 部品の組み合わせで決める（on_collide_entity_objects。石とスイッチだけ。ほかの組は以前も何も起こさなかった）
 */
void ActiveObjectManager::on_collide( core::ecs::EntityId a, core::ecs::EntityId b )
{
	ActiveObject* a0 = find_active_object( a );
	ActiveObject* a1 = find_active_object( b );

	if ( a0 && a1 )
	{
		a0->on_collide_with( a1 );
		a1->on_collide_with( a0 );
		return;
	}

	if ( a0 || a1 )
	{
		core::ecs::Entity* other = entity_manager_->find( a0 ? b : a );

		if ( other )
		{
			on_collide_with_entity_object( a0 ? * a0 : * a1, a0 ? a : b, * other, object_context_ ? object_context_->sound_manager : nullptr );
		}

		return;
	}

	core::ecs::Entity* e0 = entity_manager_->find( a );
	core::ecs::Entity* e1 = entity_manager_->find( b );

	if ( e0 && e1 )
	{
		on_collide_entity_objects( * e0, * e1 );
	}
}

void ActiveObjectManager::on_collide_with_entity_object( GameObject& object, core::ecs::EntityId object_id, core::ecs::Entity& entity, core::sound::SoundManager* sound_manager )
{
	// 動かない物（以前の StaticObject）: 以前は StaticObject::on_collide_with( GameObject* ) が相手の on_collide_with( StaticObject* ) を呼び、
	// 逆向き（相手 → StaticObject）は何もしなかった。1 組につき 1 回、相手に知らせる（プレイヤーはロケットを止める）
	if ( entity.get_component< ecs::StaticObjectComponent >() )
	{
		object.on_collide_with_static_object();
	}

	// 拾う物（以前の Medal・Rocket・Umbrella）: 相手が拾ったら（プレイヤーだけが true を返す）、この物体を死なせる。
	// 以前は Medal なら Player::on_collide_with( Medal* ) が medal->kill() を呼び、Rocket・Umbrella は自分の on_collide_with( Player* ) で kill していた
	if ( const auto* pickup = entity.get_component< ecs::PickupComponent >() )
	{
		if ( object.on_pick_up( pickup->kind ) )
		{
			ecs::EntityObject picked( & entity );
			picked.kill();

			if ( pickup->disable_contact_on_pickup && picked.get_rigid_body() )
			{
				ecs::object_body::set_no_contact_response( * picked.get_rigid_body(), true );
			}
		}
	}

	// 石（以前の Stone）: 以前は Stone::on_collide_with( GameObject* ) が相手の on_collide_with( Stone* )（今は on_collide_with_stone）を呼び、
	// 相手がプレイヤーなら Stone::on_collide_with( Player* ) が石を kill していた。今は相手が拾ったか（プレイヤーだけが true を返す）で決める。
	// 相手には石の窓口を渡す（プレイヤーは持っておいて投げる。敵は石の速度で「こちらへ飛んでくるか」を見る）
	if ( entity.get_component< ecs::ThrowableComponent >() )
	{
		if ( object.on_collide_with_stone( GameObjectRef( entity.get_entity_manager(), nullptr, static_cast< core::ecs::EntityId >( entity.get_id() ) ) ) )
		{
			ecs::EntityObject( & entity ).kill();
		}
	}

	// スイッチ（以前の BaseSwitch）: 以前は相手がプレイヤーなら BaseSwitch::on_collide_with( Player* ) が、プレイヤーの速さで入・切を変えた
	// （逆向き（スイッチ → 相手）の on_collide_with( BaseSwitch* ) を受け取る物体は無かった）。今は押せる物体か（プレイヤーだけ）で決める
	if ( entity.get_component< ecs::SwitchComponent >() && object.can_press_switch() )
	{
		if ( auto* system = entity.get_entity_manager()->get_system< ecs::SwitchSystem >() )
		{
			system->on_touch( static_cast< core::ecs::EntityId >( entity.get_id() ), object_id, object.get_velocity() );
		}
	}

	// 風船（以前の Balloon）: 以前は Balloon::on_collide_with( GameObject* ) が相手の on_collide_with( Balloon* ) を呼び、
	// 相手がプレイヤーなら、ほかの物体につかまれていない風船をつかんだ（逆向き（相手 → 風船）の on_collide_with( Player* ) などは何もしなかった）。
	// 今はつかまれていない風船だけ相手に知らせ、相手がつかんだら（プレイヤーだけが窓口を返す）、風船をつかませる
	if ( const auto* balloon = entity.get_component< ecs::BalloonHoldComponent >(); balloon && ! balloon->is_held() )
	{
		const auto balloon_id = static_cast< core::ecs::EntityId >( entity.get_id() );

		if ( ecs::BalloonHolder* holder = object.on_collide_with_balloon( balloon_id ) )
		{
			if ( auto* system = entity.get_entity_manager()->get_system< ecs::BalloonHoldSystem >() )
			{
				system->grab( balloon_id, object_id, holder );
			}
		}
	}

	// 敵（以前の Robot）: 以前は Robot と相手が互いに on_collide_with を呼び、相手がプレイヤーなら、
	// プレイヤーは追っている敵に当たると傷つき（Player::on_collide_with( Robot* )）、敵は立つ・回る間なら止まった（Robot::on_collide_with( Player* )）。
	// 2 つは別の状態でだけ起きるので、呼ぶ順によらない。今は相手に「追っているか」を知らせ、止められる相手（プレイヤーだけ true）なら敵に知らせる
	if ( entity.get_component< ecs::RobotBehaviorComponent >() )
	{
		const auto robot_id = static_cast< core::ecs::EntityId >( entity.get_id() );
		auto* system = entity.get_entity_manager()->get_system< ecs::RobotBehaviorSystem >();
		const bool is_chasing = system && system->get_mode( robot_id ) == ecs::RobotBehaviorComponent::Mode::CHASE;

		if ( object.on_collide_with_robot( is_chasing ) && system )
		{
			system->on_touched_by_target( robot_id );
		}
	}

	// 梯子（以前の Ladder）: 以前は Ladder::on_collide_with( GameObject* ) が相手の on_collide_with( Ladder* ) を呼び、
	// 相手がプレイヤーなら Ladder::on_collide_with( Player* ) が、最近（0.5 秒以内）触れていなければ梯子の位置で音を鳴らして覚えた。
	// 今は相手が反応したか（プレイヤーだけが true を返す）で決める
	if ( entity.get_component< ecs::LadderComponent >() )
	{
		if ( object.on_collide_with_ladder() )
		{
			if ( auto* cache = entity.get_component< ecs::ContactCacheComponent >() )
			{
				if ( ! cache->cache.is_cached( object_id ) )
				{
					ecs::EntityObject( & entity ).play_sound( sound_manager, "ladder-contact" );
				}

				cache->cache.cache( object_id, ecs::ContactCacheComponent::LifeTime );
			}
		}
	}
}

void ActiveObjectManager::on_collide_entity_objects( core::ecs::Entity& a, core::ecs::Entity& b )
{
	// 石と風船 : 以前は Stone::on_collide_with( GameObject* ) が風船の on_collide_with( Stone* )（今の on_collide_with_stone）を呼び、
	// 風船は割れて音を鳴らした（石は拾わない）。1 組につき 1 回
	auto burst_by_stone = [] ( core::ecs::Entity& balloon, core::ecs::Entity& stone )
	{
		if ( ! balloon.get_component< ecs::BalloonHoldComponent >() || ! stone.get_component< ecs::ThrowableComponent >() )
		{
			return false;
		}

		if ( auto* system = balloon.get_entity_manager()->get_system< ecs::BalloonHoldSystem >() )
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
		if ( ! robot.get_component< ecs::RobotBehaviorComponent >() || ! stone.get_component< ecs::ThrowableComponent >() )
		{
			return false;
		}

		if ( auto* system = robot.get_entity_manager()->get_system< ecs::RobotBehaviorSystem >() )
		{
			const ecs::EntityObject s( & stone );
			const ecs::EntityObject r( & robot );

			system->on_hit_by_stone( static_cast< core::ecs::EntityId >( robot.get_id() ), GameObject::is_moving_to( s.get_location(), s.get_velocity(), r.get_location() ) );
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
		const auto* switch_component = s.get_component< ecs::SwitchComponent >();

		if ( ! switch_component || ! switch_component->is_switched_by_stone || ! stone.get_component< ecs::ThrowableComponent >() )
		{
			return false;
		}

		if ( auto* system = s.get_entity_manager()->get_system< ecs::SwitchSystem >() )
		{
			system->on_touch( static_cast< core::ecs::EntityId >( s.get_id() ), static_cast< core::ecs::EntityId >( stone.get_id() ), ecs::EntityObject( & stone ).get_velocity() );
		}

		return true;
	};

	if ( ! press_by_stone( a, b ) )
	{
		press_by_stone( b, a );
	}
}

void ActiveObjectManager::sync_render_components() const
{
	for ( const auto* active_object : active_object_list_ )
	{
		active_object->sync_render_component();
	}
}

}; // namespace blue_sky
