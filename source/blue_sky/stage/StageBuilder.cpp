#include "StageBuilder.h"

#include <blue_sky/ObjectFactory.h>
#include <blue_sky/ActiveObjectPhysics.h>
#include <blue_sky/DebugLog.h>
#include <blue_sky/graphics/GraphicsManager.h>

#include <GameObject/Player.h>
#include <GameObject/Camera.h>

#include <core/sound/SoundManager.h>

#include <common/exception.h>

namespace blue_sky::stage
{

namespace
{

core::Vector to_vector( const Vector3d& v )
{
	return core::Vector( static_cast< float_t >( v[ 0 ] ), static_cast< float_t >( v[ 1 ] ), static_cast< float_t >( v[ 2 ] ) );
}

} // namespace

StageBuilder::Result StageBuilder::build( const StageData& data, const Context& context )
{
	Result result;

	for ( const auto& warning : data.warnings )
	{
		debug_log( "[Stage] " + warning );
	}

	// 読み込むときに 1 回だけ鳴らす BGM（以前の bgm 行）
	if ( ! data.bgm.empty() && context.sound_manager )
	{
		context.sound_manager->load_music( "bgm", data.bgm.c_str() );

		if ( data.bgm_bpm && context.set_bpm )
		{
			context.set_bpm( static_cast< float_t >( * data.bgm_bpm ) );
		}
	}

	if ( ! data.collision.empty() && context.physics )
	{
		context.physics->load_obj( ( context.stage_dir + data.collision ).c_str() );
	}

	if ( ! data.ground.empty() && context.graphics_manager )
	{
		context.graphics_manager->set_ground( data.ground.c_str() );
	}

	ObjectFactory* factory = context.object_factory;

	for ( const ObjectDesc& o : data.objects )
	{
		ObjectFactory::EntityId id = 0;

		if ( o.type == "static" )
		{
			id = factory->create_static_object( o.model, to_vector( o.position ), to_vector( o.rotation ) );
		}
		else if ( o.type == "translation" )
		{
			id = factory->create_translation_object( to_vector( o.position ), static_cast< float_t >( o.size[ 0 ] ), static_cast< float_t >( o.size[ 1 ] ), static_cast< float_t >( o.size[ 2 ] ), static_cast< float_t >( o.speed ) );
		}
		else if ( o.type == "area-switch" )
		{
			// 空間スイッチ（ECS だけの物体）
			id = factory->create_area_switch( static_cast< float_t >( o.size[ 0 ] ), static_cast< float_t >( o.size[ 1 ] ), static_cast< float_t >( o.size[ 2 ] ) );

			GameObjectRef object = factory->get_ref( id );
			object.set_start_location( static_cast< float_t >( o.position[ 0 ] ), static_cast< float_t >( o.position[ 1 ] ), static_cast< float_t >( o.position[ 2 ] ) );
			object.set_start_direction_degree( static_cast< float_t >( o.direction ) );
		}
		else
		{
			// 種類の名前から作る（ECS だけの物体になった種類も、まだ ActiveObject の種類も同じ入口）
			auto created = factory->create_object( o.type );

			if ( ! created )
			{
				COMMON_THROW_EXCEPTION_MESSAGE( o.where.to_string() + ": create object failed. ( " + o.type + " )" );
			}

			id = * created;

			GameObjectRef object = factory->get_ref( id );
			object.set_start_location( static_cast< float_t >( o.position[ 0 ] ), static_cast< float_t >( o.position[ 1 ] ), static_cast< float_t >( o.position[ 2 ] ) );
			object.set_start_direction_degree( static_cast< float_t >( o.direction ) );

			if ( o.type == "girl" )
			{
				// 少女は ECS だけの物体（以前は Girl* を結果で返し、プレイヤーを渡していたが、どちらも使っていなかった）。名前 "girl" で引く
				factory->name( "girl", id );
			}
			else if ( o.type == "robot" )
			{
				// 敵（ECS だけの物体）に追う相手を渡す（以前の Robot::set_player）
				if ( context.player )
				{
					factory->set_robot_target( id, context.player->get_entity_id(), context.player );
				}
			}
		}

		for ( const auto& name : o.names )
		{
			factory->name( name, id );
		}

		for ( const auto& e : o.events )
		{
			factory->get_ref( id ).add_event_handler( e.event.c_str(), context.make_call( e.action ) );
		}
	}

	if ( data.player && context.player )
	{
		const auto& p = data.player->position;

		context.player->set_start_location( static_cast< float_t >( p[ 0 ] ), static_cast< float_t >( p[ 1 ] ), static_cast< float_t >( p[ 2 ] ) );

		if ( context.physics )
		{
			context.player->set_rigid_body( context.physics->add_active_object_as_capsule( context.player ) );
		}

		if ( data.player->camera_direction && context.camera )
		{
			const float_t r = static_cast< float_t >( * data.player->camera_direction );

			context.camera->rotate_degree_target().set_x( r );
			context.camera->rotate_degree().set_x( r );
		}
	}

	if ( data.goal )
	{
		const auto& g = * data.goal;
		GameObjectRef goal = context.goal;
		goal.set_start_location( static_cast< float_t >( g[ 0 ] ), static_cast< float_t >( g[ 1 ] ), static_cast< float_t >( g[ 2 ] ) );
	}

	for ( const auto& a : data.start_actions )
	{
		result.start_calls.push_back( context.make_call( a ) );
	}

	result.bgm_loop = data.bgm_loop.value_or( true );

	return result;
}

} // namespace blue_sky::stage
