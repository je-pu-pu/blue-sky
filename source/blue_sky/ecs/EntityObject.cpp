#include "EntityObject.h"

#include <blue_sky/ecs/ObjectBody.h>
#include <blue_sky/ecs/Flicker.h>
#include <blue_sky/ecs/component/BalloonHoldComponent.h>
#include <blue_sky/ecs/component/FlickerComponent.h>
#include <blue_sky/ecs/component/GirlComponent.h>
#include <blue_sky/ecs/system/RobotBehaviorSystem.h>
#include <blue_sky/ecs/system/PlayerBehaviorSystem.h>
#include <blue_sky/ecs/system/PlayerInventorySystem.h>
#include <blue_sky/ecs/component/PhysicsBodyComponent.h>
#include <blue_sky/ecs/component/SimpleObjectComponent.h>
#include <blue_sky/ecs/component/CollisionTraitsComponent.h>
#include <blue_sky/ecs/component/ObjectRenderComponent.h>
#include <blue_sky/ecs/component/PickupComponent.h>
#include <blue_sky/ecs/component/ContactCacheComponent.h>
#include <blue_sky/ecs/component/SwitchComponent.h>
#include <blue_sky/ecs/component/AnimationComponent.h>
#include <core/animation/AnimationPlayer.h>
#include <blue_sky/ecs/system/PhysicsSyncSystem.h>

#include <core/ecs/Entity.h>
#include <core/ecs/EntityManager.h>
#include <core/ecs/component/TransformComponent.h>
#include <core/math/Quaternion.h>
#include <core/sound/Sound.h>
#include <core/sound/SoundManager.h>

#include <common/math.h>

#include <btBulletDynamicsCommon.h>

#include <cmath>

namespace blue_sky::ecs
{

EntityObject::EntityObject( core::ecs::Entity* entity )
	: entity_( entity )
	, transform_( entity ? entity->get_component< core::ecs::TransformComponent >() : nullptr )
	, body_( entity ? entity->get_component< PhysicsBodyComponent >() : nullptr )
	, state_( entity ? entity->get_component< SimpleObjectComponent >() : nullptr )
{

}

btRigidBody* EntityObject::get_rigid_body() const
{
	return body_ ? body_->body : nullptr;
}

void EntityObject::commit_transform()
{
	if ( get_rigid_body() )
	{
		PhysicsSyncSystem::write_body( * get_rigid_body(), body_->height_offset, transform_->transform );
	}
}

const core::Vector& EntityObject::get_location() const
{
	return transform_->transform.get_position();
}

void EntityObject::set_location( const core::Vector& location )
{
	transform_->transform.set_position( location );
	commit_transform();
}

const core::Transform& EntityObject::get_transform() const
{
	return transform_->transform;
}

core::Vector EntityObject::get_velocity() const
{
	return get_rigid_body() ? object_body::get_velocity( * get_rigid_body() ) : core::Vector::Zero;
}

void EntityObject::set_velocity( const core::Vector& velocity )
{
	if ( get_rigid_body() )
	{
		object_body::set_velocity( * get_rigid_body(), velocity );
	}
}

void EntityObject::update_velocity_by_target_location( const core::Vector& target_location, float_t speed )
{
	core::Vector relative_position = target_location - get_location();

	if ( relative_position.length() > 0.1f )
	{
		relative_position.normalize();
	}

	set_velocity( relative_position * speed );
}

void EntityObject::stop_at_target_location( const core::Vector& target_location )
{
	set_location( target_location );
	set_velocity( core::Vector::Zero );
}

void EntityObject::notify_arrive_at_target_location()
{
	if ( auto* girl = entity_->get_component< GirlComponent >() )
	{
		set_velocity( core::Vector::Zero );
		girl->float_base_location = get_location();
	}
}

float_t EntityObject::get_direction_degree() const
{
	return state_ ? state_->direction_degree : 0.f;
}

void EntityObject::set_direction_degree( float_t degree )
{
	while ( degree < 0.f ) degree += 360.f;
	while ( degree > 360.f ) degree -= 360.f;

	if ( state_ )
	{
		state_->direction_degree = degree;
	}

	core::Quaternion q{};
	q.set_yaw_pitch_roll( math::degree_to_radian( degree ), 0.f, 0.f );
	transform_->transform.set_rotation( q );

	commit_transform();
}

void EntityObject::chase_direction_degree( float_t d, float_t speed )
{
	float_t current = get_direction_degree();

	while ( current < 0.f ) current += 360.f;
	while ( current > 360.f ) current -= 360.f;

	while ( d < 0.f ) d += 360.f;
	while ( d > 360.f ) d -= 360.f;

	float_t diff = d - current;

	while ( diff < -180.f ) diff += 360.f;
	while ( diff >  180.f ) diff -= 360.f;

	set_direction_degree( math::chase( current, current + diff, speed ) );
}

void EntityObject::chase_direction_to( const core::Vector& location, float_t speed )
{
	core::Vector relative_position = location - get_location();
	relative_position.set_y( 0 );

	chase_direction_degree( math::radian_to_degree( std::atan2( relative_position.x(), relative_position.z() ) ), speed );
}

core::Vector EntityObject::get_front() const
{
	// 以前の ActiveObject::set_direction_degree と同じ計算（Y 軸の回転行列で前を回して正規化する）
	core::Matrix m;
	m.set_rotation_xyz( 0, math::degree_to_radian( get_direction_degree() ), 0 );

	core::Vector front = core::Vector::Forward * m;
	front.normalize();

	return front;
}

core::Vector EntityObject::get_right() const
{
	core::Matrix m;
	m.set_rotation_xyz( 0, math::degree_to_radian( get_direction_degree() ), 0 );

	core::Vector right = core::Vector::Right * m;
	right.normalize();

	return right;
}

core::Vector EntityObject::get_start_location() const
{
	return state_ ? state_->start_location : get_location();
}

unsigned int EntityObject::get_flicker_seed() const
{
	return static_cast< unsigned int >( entity_->get_id() );
}

void EntityObject::update_location_by_flicker( const core::Vector& base_location, float_t scale, float_t scene_elapsed_time )
{
	set_location( base_location + core::Vector( 0.f, flicker::height_offset( get_flicker_seed(), scene_elapsed_time, scale ), 0.f ) );
}

void EntityObject::update_velocity_by_flicker( const core::Vector& base_location, float_t scale, float_t scene_elapsed_time )
{
	const core::Vector target_location = base_location + core::Vector( 0.f, flicker::height_offset( get_flicker_seed(), scene_elapsed_time, scale ), 0.f );
	const core::Vector relative_position = target_location - get_location();
	const core::Vector velocity = get_velocity();

	set_velocity( core::Vector( velocity.x(), relative_position.y(), velocity.z() ) );
}

void EntityObject::set_start_location( const core::Vector& location )
{
	if ( state_ )
	{
		state_->start_location = location;
	}

	// 剛体へは送らない（以前の ActiveObject::set_start_location と同じ。剛体を作る前に呼ぶ・やり直しで送る）
	transform_->transform.set_position( location );
}

void EntityObject::set_start_rotation( float_t x, float_t y, float_t z )
{
	if ( state_ )
	{
		state_->start_rotation.set( x, y, z );
	}

	core::Quaternion q{};
	q.set_yaw_pitch_roll( math::degree_to_radian( y ), math::degree_to_radian( x ), math::degree_to_radian( z ) );
	transform_->transform.set_rotation( q );
}

void EntityObject::set_start_direction_degree( float_t degree )
{
	if ( state_ )
	{
		state_->start_direction_degree = degree;
	}
}

void EntityObject::restart()
{
	if ( auto* traits = entity_->get_component< CollisionTraitsComponent >() )
	{
		traits->is_dead = false;
	}

	if ( btRigidBody* body = get_rigid_body() )
	{
		const core::Vector start_location = state_ ? state_->start_location : get_location();
		const core::Vector start_rotation = state_ ? state_->start_rotation : core::Vector::Zero;

		core::Transform& transform = transform_->transform;

		transform.set_position( start_location );
		transform.set_rotation( core::Quaternion::from_yaw_pitch_roll( math::degree_to_radian( start_rotation.y() ), math::degree_to_radian( start_rotation.x() ), math::degree_to_radian( start_rotation.z() ) ) );

		// 以前の ActiveObject::restart と同じ順・同じ値（動きの状態・世界の位置には足元の位置を入れ、最後に向きを変えるときに高さのずれを足して送り直す）
		body->activate( true );
		body->getMotionState()->setWorldTransform( transform );

		body->setWorldTransform( transform );
		body->setInterpolationWorldTransform( transform );

		body->setLinearVelocity( btVector3( 0.f, 0.f, 0.f ) );
		body->setInterpolationLinearVelocity( btVector3( 0.f, 0.f, 0.f ) );

		body->setAngularVelocity( btVector3( 0.f, 0.f, 0.f ) );
		body->setInterpolationAngularVelocity( btVector3( 0.f, 0.f, 0.f ) );

		body->setGravity( state_ ? state_->default_gravity : core::Vector::Zero );

		body->clearForces();

		set_direction_degree( state_ ? state_->start_direction_degree : 0.f );

		// 種類ごとのやり直し（以前は派生クラスの restart）
		if ( state_ && state_->restart_mass >= 0.f )
		{
			object_body::set_mass( * body, state_->restart_mass );		// 以前の Goal::restart・Switch::restart
		}

		if ( state_ && state_->restart_angular_factor >= 0.f )
		{
			object_body::set_angular_factor( * body, state_->restart_angular_factor );	// 以前の Switch::restart
		}

		if ( state_ && state_->restart_friction >= 0.f )
		{
			object_body::set_friction( * body, state_->restart_friction );	// 以前の Switch::restart
		}

		if ( const auto* pickup = entity_->get_component< PickupComponent >(); pickup && pickup->disable_contact_on_pickup )
		{
			object_body::set_no_contact_response( * body, false );		// 以前の Rocket::restart（拾われたときに消した当たりの応答を戻す）
		}

		if ( const auto* throwable = entity_->get_component< ThrowableComponent >() )
		{
			// 以前の Stone::restart（石が薄い壁を貫通しないようにする。Bullet の連続衝突判定）
			body->setCcdSweptSphereRadius( throwable->ccd_swept_sphere_radius );
			body->setCcdMotionThreshold( throwable->ccd_motion_threshold );
		}
	}

	// スイッチは 切 に戻る（以前の BaseSwitch::restart）。通常のスイッチは "Off" のアニメーションにする（以前の Switch::restart）
	if ( auto* s = entity_->get_component< SwitchComponent >() )
	{
		s->state = SwitchComponent::State::OFF;

		if ( auto* animation = entity_->get_component< AnimationComponent >(); s->has_feedback && animation && animation->player )
		{
			animation->player->play( "Off", false, false );
		}
	}

	// 最近触れた物体を忘れる（以前の Ladder::restart・BaseSwitch::restart）
	if ( auto* cache = entity_->get_component< ContactCacheComponent >() )
	{
		cache->cache.clear();
	}

	// 風船はつかまれていない状態に戻る（以前の Balloon::restart）
	if ( auto* balloon = entity_->get_component< BalloonHoldComponent >() )
	{
		balloon->holder_id.reset();
	}

	// ゆらぎの大きさを戻す（以前の ActiveObject::restart）
	if ( auto* flicker = entity_->get_component< FlickerComponent >() )
	{
		flicker->scale = 1.f;
	}

	// 少女は立ち、浮く中心は始めの位置（以前の Girl::restart）
	if ( auto* girl = entity_->get_component< GirlComponent >() )
	{
		girl->mode = GirlComponent::Mode::STAND;
		play_animation( "Stand", true, true );
		girl->float_base_location = get_start_location();
	}

	// 敵は立ち、巡回の地点を作り直す（以前の Robot::restart）
	RobotBehaviorSystem::restart( * entity_ );

	set_visible( true, true );

	if ( auto* render = entity_->get_component< ObjectRenderComponent >() )
	{
		render->is_hidden = false;
	}

	// プレイヤーの状態を戻し、回らない・摩擦 0・質量 50 にする（以前の Player::restart の、ActiveObject::restart の後の所）
	if ( entity_->get_component< PlayerBehaviorComponent >() )
	{
		if ( auto* system = entity_->get_entity_manager()->get_system< PlayerBehaviorSystem >() )
		{
			system->restart( static_cast< core::ecs::EntityId >( entity_->get_id() ) );
		}
	}

	// プレイヤーの持ち物を空にし、望遠鏡を 1 つ持つ（以前の Player::restart の持ち物の所）
	if ( entity_->get_component< PlayerInventoryComponent >() )
	{
		if ( auto* system = entity_->get_entity_manager()->get_system< PlayerInventorySystem >() )
		{
			system->restart( static_cast< core::ecs::EntityId >( entity_->get_id() ) );
		}
	}
}

void EntityObject::kill()
{
	// プレイヤーは、音を止めて鳴らし、倒れるようにする（以前の Player::kill。スクリプトの kill も同じ）
	if ( entity_->get_component< PlayerBehaviorComponent >() )
	{
		if ( auto* system = entity_->get_entity_manager()->get_system< PlayerBehaviorSystem >() )
		{
			system->kill( static_cast< core::ecs::EntityId >( entity_->get_id() ) );
			return;
		}
	}

	if ( auto* traits = entity_->get_component< CollisionTraitsComponent >() )
	{
		traits->is_dead = true;
	}

	set_visible( false, false );

	// 風船は影響の無い所へ退く（以前の Balloon::kill）
	if ( entity_->get_component< BalloonHoldComponent >() && is_valid() )
	{
		set_location( core::Vector( 0.f, -100.f, 0.f ) );
	}
}

bool EntityObject::is_dead() const
{
	const auto* traits = entity_->get_component< CollisionTraitsComponent >();

	return traits && traits->is_dead;
}

void EntityObject::play_sound( core::sound::SoundManager* sound_manager, const char_t* name, bool loop, bool force ) const
{
	core::Sound* sound = sound_manager ? sound_manager->get_sound( name ) : nullptr;

	if ( ! sound )
	{
		return;
	}

	if ( ! force && sound->is_playing() )
	{
		return;
	}

	// 以前の GameObject::play_sound と同じ値を渡す（速度の 2 つ目に z を渡しているのも以前のまま）
	const core::Vector location = get_location();
	const core::Vector velocity = get_velocity();

	sound->set_3d_position( location.x(), location.y(), location.z() );
	sound->set_3d_velocity( velocity.x(), velocity.z(), velocity.z() );
	sound->play( loop );
}

void EntityObject::stop_sound( core::sound::SoundManager* sound_manager, const char_t* name )
{
	if ( core::Sound* sound = sound_manager ? sound_manager->get_sound( name ) : nullptr )
	{
		sound->stop();
	}
}

void EntityObject::play_animation( const char_t* name, bool force, bool loop )
{
	if ( core::AnimationPlayer* player = get_animation_player() )
	{
		player->play( name, force, loop );
	}
}

core::AnimationPlayer* EntityObject::get_animation_player() const
{
	auto* animation = entity_->get_component< AnimationComponent >();

	return animation ? animation->player.get() : nullptr;
}

void EntityObject::set_visible( bool mesh, bool line )
{
	if ( auto* render = entity_->get_component< ObjectRenderComponent >() )
	{
		render->is_mesh_visible = mesh;
		render->is_line_visible = line;
		render->is_visible = mesh || line;
	}
}

} // namespace blue_sky::ecs
