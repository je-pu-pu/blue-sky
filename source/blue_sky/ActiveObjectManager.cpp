#include "ActiveObjectManager.h"
#include "ActiveObjectPhysics.h"
#include "DebugLog.h"

#include <GameObject/Girl.h>
#include <GameObject/Robot.h>
#include <GameObject/Balloon.h>
#include <blue_sky/GameObjectRef.h>
#include <GameObject/Switch.h>
#include <GameObject/AreaSwitch.h>

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
	object_creator_map_[ "girl"        ] = [] () -> ActiveObject* { return new Girl();         };
	object_creator_map_[ "robot"       ] = [this] () -> ActiveObject* { return new Robot( graphics_manager_ ); };	// テクスチャを得る GraphicsManager を渡す
	object_creator_map_[ "balloon"     ] = [] () -> ActiveObject* { return new Balloon();      };
	object_creator_map_[ "switch"      ] = [] () -> ActiveObject* { return new Switch();       };

	// "static"（動かない物）・"medal"・"umbrella"・"rocket"・"goal"・"ladder"・"stone" は ECS だけの物体になり、ObjectFactory が作る（段階 4）

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
 * - 両方とも ECS だけの物体 : 何もしない（以前も、動かない物どうしの当たりは何も起こさなかった）
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

void ActiveObjectManager::sync_render_components() const
{
	for ( const auto* active_object : active_object_list_ )
	{
		active_object->sync_render_component();
	}
}

}; // namespace blue_sky
