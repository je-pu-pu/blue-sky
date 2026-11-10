#include "ActiveObjectManager.h"
#include "ActiveObjectPhysics.h"
#include "DebugLog.h"

#include <GameObject/Girl.h>
#include <GameObject/Robot.h>
#include <GameObject/Goal.h>
#include <GameObject/Balloon.h>
#include <GameObject/Medal.h>
#include <GameObject/Ladder.h>
#include <GameObject/Rocket.h>
#include <GameObject/Umbrella.h>
#include <GameObject/Stone.h>
#include <GameObject/Switch.h>
#include <GameObject/AreaSwitch.h>
#include <GameObject/TranslationObject.h>

#include <GameObject/ActiveObject.h>
#include <GameObject/StaticObject.h>
#include <GameObject/GameObjectContext.h>

#include <algorithm>

#include <blue_sky/GameMain.h>
#include <blue_sky/graphics/GraphicsManager.h>

#include <blue_sky/ecs/system/ObjectRenderDataSystem.h>
#include <blue_sky/ecs/system/MoveToTargetSystem.h>
#include <blue_sky/ecs/system/CollisionEventSystem.h>
#include <blue_sky/ecs/component/ActiveObjectLinkComponent.h>

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
	object_creator_map_[ "goal"        ] = [] () -> ActiveObject* { return new Goal();         };
	object_creator_map_[ "balloon"     ] = [] () -> ActiveObject* { return new Balloon();      };
	object_creator_map_[ "medal"       ] = [] () -> ActiveObject* { return new Medal();        };
	object_creator_map_[ "ladder"      ] = [] () -> ActiveObject* { return new Ladder();       };
	object_creator_map_[ "rocket"      ] = [] () -> ActiveObject* { return new Rocket();       };
	object_creator_map_[ "umbrella"    ] = [] () -> ActiveObject* { return new Umbrella();     };
	object_creator_map_[ "stone"       ] = [] () -> ActiveObject* { return new Stone();        };
	object_creator_map_[ "switch"      ] = [] () -> ActiveObject* { return new Switch();       };
	object_creator_map_[ "static"      ] = [] () -> ActiveObject* { return new StaticObject(); };

	// object_creator_map_[ "area-switch"        ] = [] () -> ActiveObject* { return 0; };
	// object_creator_map_[ "translation-object" ] = [] () -> ActiveObject* { return 0; };
	
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

	// 目標（部品）は、物体を壊すときに Entity ごと消える
	active_object_list_.clear();
	named_active_object_map_.clear();
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

	// Entity を作り、描画の部品と定数バッファを用意する（以前は ActiveObject が定数バッファ・Model* を自分で持っていた）
	active_object->attach_entity( entity_manager_, graphics_manager_ );

	active_object_list_.push_back( active_object );
}

void ActiveObjectManager::name_active_object( const string_t& name, ActiveObject* active_object )
{
	named_active_object_map_[ name ] = active_object;
}

void ActiveObjectManager::set_target_location( ActiveObject* active_object, const Vector& target_location, float_t speed )
{
	auto* system = entity_manager_->get_system< ecs::MoveToTargetSystem >();

	if ( ! system || ! active_object->has_entity() || ! system->set_target_location( active_object->get_entity_id(), target_location, speed ) )
	{
		debug_log( "[Object] 目標地点を設定できません（物体の Entity が無い。ActiveObjectManager に加えていない・片付けた後）" );
		return;
	}

	const Vector& from = active_object->get_location();
	debug_log( "[Object] 目標地点へ動かします ( " + std::to_string( from.x() ) + ", " + std::to_string( from.y() ) + ", " + std::to_string( from.z() ) + " ) → ( "
		+ std::to_string( target_location.x() ) + ", " + std::to_string( target_location.y() ) + ", " + std::to_string( target_location.z() ) + " ) 速さ " + std::to_string( speed ) );
}

void ActiveObjectManager::set_target_direction( ActiveObject* active_object, float_t target_direction, float_t speed )
{
	auto* system = entity_manager_->get_system< ecs::MoveToTargetSystem >();

	if ( ! system || ! active_object->has_entity() || ! system->set_target_direction( active_object->get_entity_id(), target_direction, speed ) )
	{
		debug_log( "[Object] 目標の向きを設定できません（物体の Entity が無い）" );
	}
}

void ActiveObjectManager::set_target_direction_object( ActiveObject* active_object, const ActiveObject* target_active_object, float_t speed )
{
	auto* system = entity_manager_->get_system< ecs::MoveToTargetSystem >();

	if ( ! system || ! active_object->has_entity() || ! target_active_object->has_entity()
		|| ! system->set_target_direction_entity( active_object->get_entity_id(), target_active_object->get_entity_id(), speed ) )
	{
		debug_log( "[Object] 向く先の物体を設定できません（物体の Entity が無い）" );
	}
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

/**
 * 文字列によりオブジェクトを生成する
 *
 * @param ss オブジェクトを生成するためのパラメータ
 * @return 生成したオブジェクト
 */
ActiveObject* ActiveObjectManager::create_static_object( std::stringstream& ss )
{
	string_t object_name;
	float_t x = 0, y = 0, z = 0, rx = 0, ry = 0, rz = 0;

	ss >> object_name >> x >> y >> z >> rx >> ry >> rz;

	std::unordered_map< string_t, Vector > size_map;
	size_map[ "soda-can-1"   ] = Vector(  0.07f, 0.12f, 0.07f );
	size_map[ "wall-1"       ] = Vector(  4.f,   1.75f, 0.2f  );
	size_map[ "wall-2"       ] = Vector(  8.f,   2.5f,  0.2f  );
	size_map[ "wall-3"       ] = Vector(  4.f,   2.5f,  0.2f  );
	size_map[ "outdoor-unit" ] = Vector(  0.7f,  0.6f,  0.24f );
	size_map[ "building-20"  ] = Vector( 10.f,  20.f,  10.f   );
	size_map[ "building-200" ] = Vector( 80.f, 200.f,  60.f   );
	size_map[ "board-1"      ] = Vector(  4.f,   0.2f,  0.8f  );

	size_map[ "box-5x5x5"    ] = Vector(  5.f,  5.f,  5.f );
	size_map[ "box-2x2x2"    ] = Vector(  2.f,  2.f,  2.f );

	size_map[ "robot-dead-body" ] = Vector( 0.8f, 2.f, 0.5f );

	std::unordered_map< string_t, float_t > mass_map;
	mass_map[ "soda-can-1"   ] = 50.f;
	mass_map[ "board-1"      ] = 20.f;
	mass_map[ "box-5x5x5"    ] = 1.f;
	mass_map[ "box-2x2x2"    ] = 100.f;
	mass_map[ "robot-dead-body" ] = 10.f;
	mass_map[ "wall-3"       ] = 100.f;


	float w = 0.f, h = 0.f, d = 0.f, mass = 0.f;
			
	{
		auto i = size_map.find( object_name );

		if ( i != size_map.end() )
		{
			w = i->second.x();
			h = i->second.y();
			d = i->second.z();
		}
	}

	{
		auto i = mass_map.find( object_name );

		if ( i != mass_map.end() )
		{
			mass = i->second;
		}
	}

	ActiveObject* object = new StaticObject( w, h, d, mass );
	object->set_start_location( x, y, z );
	object->set_start_rotation( rx, ry, rz );
	object->set_start_direction_degree( rx );

	if ( object_name == "soda-can-1" )
	{
		object->set_rigid_body( physics_->add_active_object_as_cylinder( object ) );
	}
	else
	{
		object->set_rigid_body( physics_->add_active_object_as_box( object ) );
	}

	object->set_mass( mass );
	add_active_object( object );

	if ( object_name == "box-2x2x2" )
	{
		object->set_friction( 10.f );
	}

	object->set_model( graphics_manager_->load_model( object_name.c_str() ) );

	return object;
}

ActiveObject* ActiveObjectManager::get_active_object( const string_t& name )
{
	auto i = named_active_object_map_.find( name );

	if ( i == named_active_object_map_.end() )
	{
		return 0;
	}

	return i->second;
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

	// 目標地点・向きへ動かす（以前はここで 3 つの unordered_map を回していた。今は部品を Entity の番号の順に回す）
	if ( auto* system = entity_manager_->get_system< ecs::MoveToTargetSystem >() )
	{
		const float_t delta_time = object_context_ && object_context_->delta_time ? object_context_->delta_time() : 0.f;

		system->update_targets( delta_time );
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

/// 以前の ActiveObjectContactResultCallback::on_collide の「両方が物体」の所（同じ順に両方へ知らせる）
void ActiveObjectManager::on_collide( core::ecs::EntityId a, core::ecs::EntityId b )
{
	ActiveObject* a0 = find_active_object( a );
	ActiveObject* a1 = find_active_object( b );

	if ( ! a0 || ! a1 )
	{
		return;
	}

	a0->on_collide_with( a1 );
	a1->on_collide_with( a0 );
}

void ActiveObjectManager::sync_render_components() const
{
	for ( const auto* active_object : active_object_list_ )
	{
		active_object->sync_render_component();
	}
}

}; // namespace blue_sky
