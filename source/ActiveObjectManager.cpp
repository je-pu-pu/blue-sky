#include "ActiveObjectManager.h"
#include "ActiveObjectPhysics.h"

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
#include <GameObject/StaticObject.h>
#include <GameObject/AreaSwitch.h>
#include <GameObject/TranslationObject.h>

#include <GameObject/ActiveObject.h>
#include <GameObject/StaticObject.h>

#include <GameMain.h>

#include <blue_sky/graphics/GraphicsManager.h>

#include <core/animation/AnimationPlayer.h>

namespace blue_sky
{

ActiveObjectManager::ActiveObjectManager()
{
	object_creator_map_[ "girl"        ] = [] () -> ActiveObject* { return new Girl();         };
	object_creator_map_[ "robot"       ] = [] () -> ActiveObject* { return new Robot();        };
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

	active_object_list_.clear();
	named_active_object_map_.clear();
	target_location_map_.clear();
	target_direction_map_.clear();
	target_direction_object_map_.clear();
}

void ActiveObjectManager::restart()
{
	for ( auto i = active_object_list_.begin(); i != active_object_list_.end(); ++i )
	{
		( *i )->restart();
	}

	target_location_map_.clear();
	target_direction_map_.clear();
	target_direction_object_map_.clear();
}

void ActiveObjectManager::add_active_object( ActiveObject* active_object )
{
	active_object_list_.insert( active_object );
}

void ActiveObjectManager::name_active_object( const string_t& name, ActiveObject* active_object )
{
	named_active_object_map_[ name ] = active_object;
}

void ActiveObjectManager::set_target_location( ActiveObject* active_object, const Vector& target_location, float_t speed )
{
	target_location_map_[ active_object ] = std::make_tuple( target_location, speed );
}

void ActiveObjectManager::set_target_direction( ActiveObject* active_object, float_t target_direction, float_t speed )
{
	target_direction_map_[ active_object ] = std::make_tuple( target_direction, speed );
	target_direction_object_map_.erase( active_object );
}

void ActiveObjectManager::set_target_direction_object( ActiveObject* active_object, const ActiveObject* target_active_object, float_t speed )
{
	target_direction_object_map_[ active_object ] = std::make_tuple( target_active_object, speed );
	target_direction_map_.erase( active_object );
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
		object->set_rigid_body( GameMain::get_instance()->get_physics_manager()->add_active_object_as_cylinder( object ) );
	}
	else
	{
		object->set_rigid_body( GameMain::get_instance()->get_physics_manager()->add_active_object_as_box( object ) );
	}

	object->set_mass( mass );
	add_active_object( object );

	if ( object_name == "box-2x2x2" )
	{
		object->set_friction( 10.f );
	}

	object->set_model( GameMain::get_instance()->get_graphics_manager()->load_model( object_name.c_str() ) );

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
	for ( auto i = active_object_list_.begin(); i != active_object_list_.end(); ++i )
	{
		// if ( ! (*i)->is_dead() )
		{
			(*i)->update();
		}

		if ( ( *i )->get_animation_player() )
		{
			( *i )->get_animation_player()->update();
		}
	}

	for ( auto i = target_location_map_.begin(); i != target_location_map_.end(); )
	{
		i->first->update_velocity_by_target_location( std::get< 0 >( i->second ), std::get< 1 >( i->second ) );
		// i->first->set_location( std::get< 0 >( i->second ) );

		/// @todo 目的座標に到達したかの判定をもう少し正確にする
		if ( ( std::get< 0 >( i->second ) - i->first->get_location() ).length() < 0.1f )
		{
			i->first->on_arrive_at_target_location();

			i = target_location_map_.erase( i );
		}
		else
		{
			++i;
		}
	}

	for ( auto i = target_direction_map_.begin(); i != target_direction_map_.end(); ++i )
	{
		i->first->chase_direction_degree( std::get< 0 >( i->second ), std::get< 1 >( i->second ) );
	}

	for ( auto i = target_direction_object_map_.begin(); i != target_direction_object_map_.end(); ++i )
	{
		i->first->chase_direction_to( std::get< 0 >( i->second )->get_location(), std::get< 1 >( i->second ) );
	}
}

/**
 * 全ての ActiveObject を描画する
 *
 */
/*
void ActiveObjectManager::render()
{

}
*/

}; // namespace blue_sky