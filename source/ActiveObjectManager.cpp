#include "ActiveObjectManager.h"
#include "ActiveObject.h"
#include "AnimationPlayer.h"

#include "StaticObject.h"
#include "DrawingModelManager.h"
#include "ActiveObjectPhysics.h"

namespace blue_sky
{

ActiveObjectManager::ActiveObjectManager()
{
	
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

void ActiveObjectManager::set_target_location( ActiveObject* active_object, const Vector3& target_location, float_t speed )
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
 * 文字列によりオブジェクトを生成する
 *
 */
ActiveObject* ActiveObjectManager::create_object( std::stringstream& ss, DrawingModelManager* drawing_model_manager, ActiveObjectPhysics* physics )
{
	string_t object_name;
	float_t x = 0, y = 0, z = 0, rx = 0, ry = 0, rz = 0;

	ss >> object_name >> x >> y >> z >> rx >> ry >> rz;

	std::map< string_t, ActiveObject::Vector3 > size_map;
	size_map[ "soda-can-1"   ] = ActiveObject::Vector3(  0.07f, 0.12f, 0.07f );
	size_map[ "wall-1"       ] = ActiveObject::Vector3(  4.f,   1.75f, 0.2f  );
	size_map[ "wall-2"       ] = ActiveObject::Vector3(  8.f,   2.5f,  0.2f  );
	size_map[ "wall-3"       ] = ActiveObject::Vector3(  4.f,   2.5f,  0.2f  );
	size_map[ "outdoor-unit" ] = ActiveObject::Vector3(  0.7f,  0.6f,  0.24f );
	size_map[ "building-20"  ] = ActiveObject::Vector3( 10.f,  20.f,  10.f   );
	size_map[ "building-200" ] = ActiveObject::Vector3( 80.f, 200.f,  60.f   );
	size_map[ "board-1"      ] = ActiveObject::Vector3(  4.f,   0.2f,  0.8f  );

	size_map[ "box-5x5x5"    ] = ActiveObject::Vector3(  5.f,  5.f,  5.f );
	size_map[ "box-2x2x2"    ] = ActiveObject::Vector3(  2.f,  2.f,  2.f );

	size_map[ "robot-dead-body" ] = ActiveObject::Vector3( 0.8f, 2.f, 0.5f );

	std::map< string_t, float_t > mass_map;
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
		object->set_rigid_body( physics->add_active_object_as_cylinder( object ) );
	}
	else
	{
		object->set_rigid_body( physics->add_active_object( object ) );
	}

	object->set_mass( mass );
	add_active_object( object );

	if ( object_name == "box-2x2x2" )
	{
		object->get_rigid_body()->setFriction( 10 );
	}

	object->set_drawing_model( drawing_model_manager->load( object_name.c_str() ) );
	object->setup_animation_player();

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
void ActiveObjectManager::render()
{

}

}; // namespace blue_sky