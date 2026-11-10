#include "RobotBehaviorSystem.h"

#include <blue_sky/graphics/ResourceLoader.h>
#include <blue_sky/graphics/Model.h>

#include <blue_sky/ecs/EntityObject.h>
#include <blue_sky/ecs/ObjectBody.h>
#include <blue_sky/ecs/Flicker.h>
#include <blue_sky/ecs/component/FlickerComponent.h>
#include <blue_sky/ecs/component/ObjectRenderComponent.h>
#include <blue_sky/ecs/component/PatrolComponent.h>
#include <blue_sky/ecs/system/CollisionEventSystem.h>

#include <core/animation/AnimationPlayer.h>
#include <core/ecs/EntityManager.h>
#include <core/graphics/Shader.h>

#include <common/math.h>

#include <core/physics/PhysicsManager.h>

#include <algorithm>
#include <cmath>

namespace blue_sky::ecs
{

namespace
{

/// 見た目ごとのテクスチャの名前（以前の Robot のコンストラクタ）
constexpr const char_t* TextureNames[] = { "robot", "robot-warn", "robot-error", "robot-tender" };

} // namespace

RobotBehaviorSystem::RobotBehaviorSystem( const core::physics::PhysicsManager* physics, graphics::ResourceLoader* resources, core::sound::SoundManager* sound_manager )
	: physics_( physics )
	, resources_( resources )
	, sound_manager_( sound_manager )
{

}

RobotBehaviorComponent* RobotBehaviorSystem::find_robot( core::ecs::EntityId id ) const
{
	core::ecs::Entity* entity = get_entity_manager()->find( id );

	return entity ? entity->get_component< RobotBehaviorComponent >() : nullptr;
}

std::optional< EntityObject > RobotBehaviorSystem::find_target( const RobotBehaviorComponent& robot ) const
{
	core::ecs::Entity* entity = robot.target_id ? get_entity_manager()->find( * robot.target_id ) : nullptr;

	if ( ! entity )
	{
		return std::nullopt;
	}

	EntityObject target( entity );

	return target.is_valid() ? std::optional< EntityObject >( target ) : std::nullopt;
}

void RobotBehaviorSystem::load_textures( core::ecs::EntityId id )
{
	RobotBehaviorComponent* robot = find_robot( id );

	if ( ! robot || ! resources_ )
	{
		return;
	}

	for ( int i = 0; i < static_cast< int >( Look::MAX ); i++ )
	{
		// 名前のテクスチャが無ければ media/model/<名前>.png を読んで登録する（ResourceLoader::get_texture）。無ければ空のハンドル
		resources_->get_texture( TextureNames[ i ] );
		robot->textures[ i ] = resources_->get_texture_handle( TextureNames[ i ] );
	}
}

void RobotBehaviorSystem::set_target( core::ecs::EntityId id, core::ecs::EntityId target_id )
{
	if ( RobotBehaviorComponent* robot = find_robot( id ) )
	{
		robot->target_id = target_id;
	}
}

void RobotBehaviorSystem::restart( core::ecs::Entity& entity )
{
	auto* robot = entity.get_component< RobotBehaviorComponent >();

	if ( ! robot )
	{
		return;
	}

	EntityObject object( & entity );

	robot->mode = Mode::STAND;
	robot->timer = 0.f;
	robot->look = Look::NORMAL;

	object.play_animation( "Stand", false, true );

	// 巡回の地点 : 始めの位置と、その 5 m 前（以前の Robot::restart）
	if ( auto* patrol = entity.get_component< PatrolComponent >() )
	{
		patrol->waypoints.clear();
		patrol->current = 0;

		patrol->waypoints.push_back( object.get_location() );
		patrol->waypoints.push_back( object.get_location() + ( object.get_front() * RobotBehaviorComponent::PatrolForwardDistance ) );
	}
}

void RobotBehaviorSystem::update_robots( float_t delta_time, float_t scene_elapsed_time )
{
	id_list_.clear();

	for ( const auto& i : get_component_list() )
	{
		id_list_.push_back( i.first );
	}

	std::sort( id_list_.begin(), id_list_.end() );

	for ( const core::ecs::EntityId id : id_list_ )
	{
		update_robot( id, delta_time, scene_elapsed_time );
	}
}

void RobotBehaviorSystem::update_robot( core::ecs::EntityId id, float_t delta_time, float_t scene_elapsed_time )
{
	core::ecs::Entity* entity = get_entity_manager()->find( id );
	RobotBehaviorComponent* robot = entity ? entity->get_component< RobotBehaviorComponent >() : nullptr;

	if ( ! robot )
	{
		return;
	}

	EntityObject object( entity );

	if ( ! object.is_valid() )
	{
		return;
	}

	if ( core::physics::RigidBody* body = object.get_rigid_body() )
	{
		body->set_activation_state_active();
	}

	const std::optional< EntityObject > target = find_target( * robot );

	if ( robot->mode == Mode::CHASE )
	{
		if ( target )
		{
			// ターゲットの方を向く ( 表示上の向き )
			object.chase_direction_to( target->get_location(), 2.f );

			// ターゲットとの距離が遠い場合は、ターゲットの進行方向を予測して進む
			core::Vector relative_position = target->get_location() - object.get_location();
			relative_position.set_y( 0 );

			core::Vector target_xz_velocity = target->get_velocity();
			target_xz_velocity.set_y( 0 );

			if ( relative_position.length() > 3.f && target_xz_velocity.length() > 1.f )
			{
				relative_position = ( target->get_location() + target->get_front() * 3.f ) - object.get_location();
				relative_position.set_y( 0 );
			}

			// 進行方向を設定
			core::Vector forward = relative_position;
			forward.normalize();

			object.set_velocity( core::Vector( forward.x() * 3.f, object.get_velocity().y(), forward.z() * 3.f ) );
		}

		object.play_animation( "Walk", false, true );

		robot->look = flicker::is_visible_in_blink( scene_elapsed_time, 2.f ) ? Look::ALARM : Look::WARNING;

		robot->timer += delta_time;

		if ( robot->timer >= 10.f )
		{
			if ( ! is_target_visible( id ) )
			{
				robot->mode = Mode::ROTATION;
				robot->timer = 0.f;
				object.play_sound( sound_manager_, "robot-shutdown" );
				EntityObject::stop_sound( sound_manager_, "robot-chase" );

				robot->look = Look::NORMAL;
			}
		}
		else
		{
			object.play_sound( sound_manager_, "robot-chase", false, false );
		}
	}
	else if ( robot->mode == Mode::STAND || robot->mode == Mode::ROTATION )
	{
		object.set_velocity( core::Vector( 0.f, 0.f, 0.f ) );

		if ( robot->mode == Mode::ROTATION )
		{
			object.set_direction_degree( object.get_direction_degree() + 0.25f );
		}

		object.play_animation( "Stand", false, true );

		if ( is_target_visible( id ) )
		{
			robot->mode_backup = robot->mode;
			robot->mode = Mode::ATTENTION;
			robot->timer = 0.f;

			object.play_sound( sound_manager_, "robot-found", false, false );
			robot->look = Look::WARNING;
		}
	}
	else if ( robot->mode == Mode::PATROL )
	{
		// 以前の Robot::update_patrol（今の地点へ向かうだけで、次の地点へは進まない。以前と同じ）
		if ( const auto* patrol = entity->get_component< PatrolComponent >(); patrol && patrol->current < patrol->waypoints.size() )
		{
			object.update_velocity_by_target_location( patrol->waypoints[ patrol->current ], patrol->move_speed );

			object.play_animation( "Walk", false, true );
		}
	}
	else if ( robot->mode == Mode::ATTENTION )
	{
		object.set_velocity( core::Vector( 0.f, 0.f, 0.f ) );

		if ( target )
		{
			// ターゲットの方を向く
			core::Vector relative_position = target->get_location() - object.get_location();
			relative_position.set_y( 0 );

			// 表示上の向きを設定
			const float_t target_direction_degree = math::radian_to_degree( std::atan2( relative_position.x(), relative_position.z() ) );
			object.chase_direction_degree( target_direction_degree, 2.f );
		}

		object.play_animation( "Attention", false, true );

		robot->timer += delta_time;

		if ( robot->timer >= 3.f )
		{
			if ( is_target_visible( id ) )
			{
				robot->mode = Mode::CHASE;
				robot->timer = 0.f;
				object.play_sound( sound_manager_, "robot-chase-start", false, false );
			}
			else
			{
				robot->mode = robot->mode_backup;
				robot->look = Look::NORMAL;

				robot->timer = 0.f;
				object.play_sound( sound_manager_, "robot-shutdown" );
			}
		}
	}
	else if ( robot->mode == Mode::SHUTDOWN )
	{
		object.set_velocity( object.get_velocity() * 0.5f );
		robot->look = Look::NORMAL;

		if ( const core::AnimationPlayer* animation = object.get_animation_player(); animation && animation->get_current_frame() >= 60 )
		{
			// 倒れた敵に引っかからないようにする（やり直しても戻さない。以前と同じ）
			if ( core::physics::RigidBody* body = object.get_rigid_body() )
			{
				object_body::set_no_contact_response( * body, true );
			}
		}
	}
	else if ( robot->mode == Mode::FLOAT )
	{
		const auto* flicker = entity->get_component< FlickerComponent >();
		object.update_velocity_by_flicker( object.get_start_location(), flicker ? flicker->scale : 1.f, scene_elapsed_time );

		if ( target )
		{
			object.chase_direction_to( target->get_location(), 0.5f );
		}

		if ( core::AnimationPlayer* animation = object.get_animation_player() )
		{
			animation->set_speed( 0.25f );
		}

		object.play_animation( "Float", false, true );
		robot->look = Look::TENDER;
	}

	apply_look( * entity, * robot );
}

void RobotBehaviorSystem::apply_look( core::ecs::Entity& entity, const RobotBehaviorComponent& robot ) const
{
	if ( ! resources_ )
	{
		return;
	}

	const auto* render = entity.get_component< ObjectRenderComponent >();
	graphics::Model* model = render ? resources_->resolve_model( render->model ) : nullptr;

	if ( ! model || ! model->get_shader_at( 0 ) )
	{
		return;
	}

	// 解放されたテクスチャ・読めなかったテクスチャなら nullptr を設定する（以前も読めなければ nullptr だった）
	model->get_shader_at( 0 )->set_texture_at( 0, resources_->resolve_texture( robot.textures[ static_cast< int >( robot.look ) ] ) );
}

bool RobotBehaviorSystem::action( core::ecs::EntityId id, const string_t& action )
{
	RobotBehaviorComponent* robot = find_robot( id );

	if ( ! robot )
	{
		return false;
	}

	if ( action == "rotation" )
	{
		robot->mode = Mode::ROTATION;
	}
	else if ( action == "patrol" )
	{
		robot->mode = Mode::PATROL;
	}
	else if ( action == "float" )
	{
		robot->mode = Mode::FLOAT;
	}
	else
	{
		return false;
	}

	return true;
}

void RobotBehaviorSystem::shutdown( core::ecs::EntityId id )
{
	core::ecs::Entity* entity = get_entity_manager()->find( id );
	RobotBehaviorComponent* robot = entity ? entity->get_component< RobotBehaviorComponent >() : nullptr;

	if ( ! robot )
	{
		return;
	}

	EntityObject object( entity );

	robot->mode = Mode::SHUTDOWN;

	EntityObject::stop_sound( sound_manager_, "robot-found" );
	EntityObject::stop_sound( sound_manager_, "robot-chase" );

	object.play_sound( sound_manager_, "robot-shutdown", false, false );

	object.play_animation( "Shutdown", false, false );

	object.set_velocity( core::Vector::Zero );
}

void RobotBehaviorSystem::on_touched_by_target( core::ecs::EntityId id )
{
	const RobotBehaviorComponent* robot = find_robot( id );

	if ( ! robot || ( robot->mode != Mode::STAND && robot->mode != Mode::ROTATION ) )
	{
		return;
	}

	shutdown( id );
	EntityObject( get_entity_manager()->find( id ) ).play_sound( sound_manager_, "switch-off", false, false );
}

void RobotBehaviorSystem::on_hit_by_stone( core::ecs::EntityId id, bool is_stone_coming )
{
	const RobotBehaviorComponent* robot = find_robot( id );

	if ( ! robot || ( robot->mode != Mode::STAND && robot->mode != Mode::ROTATION ) )
	{
		return;
	}

	if ( is_stone_coming )
	{
		shutdown( id );
		EntityObject( get_entity_manager()->find( id ) ).play_sound( sound_manager_, "switch-off", false, false );
	}
}

std::optional< RobotBehaviorSystem::Mode > RobotBehaviorSystem::get_mode( core::ecs::EntityId id ) const
{
	if ( const RobotBehaviorComponent* robot = find_robot( id ) )
	{
		return robot->mode;
	}

	return std::nullopt;
}

bool RobotBehaviorSystem::is_target_visible( core::ecs::EntityId id ) const
{
	core::ecs::Entity* entity = get_entity_manager()->find( id );
	const RobotBehaviorComponent* robot = entity ? entity->get_component< RobotBehaviorComponent >() : nullptr;
	const std::optional< EntityObject > target = robot ? find_target( * robot ) : std::nullopt;

	if ( ! target )
	{
		return false;
	}

	EntityObject object( entity );

	const core::Vector target_location = target->get_location();
	const core::Vector relative_position = target_location - object.get_location();

	const auto relative_length = relative_position.length(); // ターゲットとの距離

	/// 視野の距離
	const float_t eyeshot_length_short = 3.f;
	const float_t eyeshot_length_middle = 10.f;
	const float_t eyeshot_length_long = 20.f;

	/// 視野の距離に応じた視野角 ( -1.f .. 1.f ) ( 1.f : 正面 / 0.f : 真横 / -1.f : 真後ろ  )
	const float_t eyeshot_angle_short = -0.25f;
	const float_t eyeshot_angle_middle = 0.25f;
	const float_t eyeshot_angle_long = 0.75f;

	if ( relative_length > eyeshot_length_long )
	{
		return false;
	}

	// ロボットの目の位置
	core::Vector from = object.get_location();
	from.set_y( from.y() + 1.5f ); // 目線の高さを追加

	// ターゲットの目の位置
	core::Vector to = target_location;
	to.set_y( to.y() + 1.5f ); // 目線の高さを追加

	float_t eyeshot_angle = 0.f;

	if ( relative_length > eyeshot_length_middle )
	{
		eyeshot_angle = eyeshot_angle_long;
	}
	else if ( relative_length > eyeshot_length_short )
	{
		eyeshot_angle = eyeshot_angle_middle;
	}
	else
	{
		eyeshot_angle = eyeshot_angle_short;
	}

	// ターゲットとの距離に応じた視野角に入っていなければ、目視できない
	if ( object.get_front().dot( ( to - from ).normalize() ) < eyeshot_angle )
	{
		return false;
	}

	if ( ! physics_ )
	{
		return true;
	}

	// 障害物判定（自分と相手を通り抜ける）
	const core::ecs::EntityManager* entity_manager = get_entity_manager();
	const auto hit = physics_->ray_test_closest( from, to, object.get_rigid_body(), target->get_rigid_body(), [ entity_manager ] ( std::optional< core::ecs::EntityId > tag ) { return CollisionEventSystem::is_ray_target( entity_manager, tag, false ); } );

	// ターゲットとの間に障害物がなければ目視できている
	return ! hit.hit;
}

} // namespace blue_sky::ecs
