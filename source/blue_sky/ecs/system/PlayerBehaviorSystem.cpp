#include "PlayerBehaviorSystem.h"

#include <blue_sky/ActiveObjectPhysics.h>
#include <blue_sky/GameObjectRef.h>
#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/graphics/Model.h>
#include <blue_sky/graphics/Line.h>

#include <blue_sky/ecs/EntityObject.h>
#include <blue_sky/ecs/ObjectBody.h>
#include <blue_sky/ecs/component/CollisionTraitsComponent.h>
#include <blue_sky/ecs/component/FlickerComponent.h>
#include <blue_sky/ecs/component/ObjectRenderComponent.h>
#include <blue_sky/ecs/component/PickupComponent.h>
#include <blue_sky/ecs/component/SimpleObjectComponent.h>
#include <blue_sky/ecs/system/CollisionEventSystem.h>

#include <core/ecs/EntityManager.h>
#include <core/sound/Sound.h>
#include <core/sound/SoundManager.h>

#include <common/math.h>

#include <btBulletDynamicsCommon.h>

#include <algorithm>
#include <cmath>

namespace blue_sky::ecs
{

/// 1 人のプレイヤーを動かすときに使う部品と窓口（find_self で作る。部品の場所は外すまで動かないので、使い終わるまで Entity を消さなければよい）
struct PlayerBehaviorSystem::Self
{
	core::ecs::Entity*			entity = nullptr;
	PlayerBehaviorComponent*	p = nullptr;
	EntityObject				object{ nullptr };
	btRigidBody*				body = nullptr;		///< 剛体（無ければ nullptr。ステージを読む前）
	float_t						delta_time = 0.f;
	float_t						scene_elapsed_time = 0.f;

	const core::Vector& location() const { return object.get_location(); }
	core::Vector velocity() const { return object.get_velocity(); }
	core::Vector front() const { return object.get_front(); }
	core::Vector right() const { return object.get_right(); }
};

namespace
{

using core::Vector;

/**
 * 自分以外の、壁・障害物になる物に触れたかを調べる（以前の Player.cpp の WithoutMeContactResultCallback と同じ）
 *
 * 当たった剛体の性質は、userPointer の Entity の番号のタグから引く（ecs::CollisionTraitsComponent）。
 * 自分は剛体で見分ける。タグの無い剛体（地面・ステージの当たりの形）は壁として当たる。タグはあるが Entity が消えた剛体は当たらない
 */
class WithoutMeContactResultCallback : public btCollisionWorld::ContactResultCallback
{
private:
	const btCollisionObject* me_;
	const core::ecs::EntityManager* entity_manager_;
	bool is_hit_ = false;

	/// 壁・障害物として数えない物か
	bool is_ignored( const btCollisionObject* object ) const
	{
		if ( object == me_ )
		{
			return true;
		}

		if ( CollisionEventSystem::is_vanished( entity_manager_, object ) )
		{
			return true;
		}

		const auto* traits = CollisionEventSystem::find_traits( entity_manager_, object );

		return traits && ! traits->is_block;
	}

public:
	WithoutMeContactResultCallback( const btCollisionObject* me, const core::ecs::EntityManager* entity_manager )
		: me_( me )
		, entity_manager_( entity_manager )
	{

	}

	btScalar addSingleResult( btManifoldPoint& /* cp */, const btCollisionObjectWrapper* o1, int /* partId0 */, int /* index0 */, const btCollisionObjectWrapper* o2, int /* partId1 */, int /* index1 */ ) override
	{
		if ( is_ignored( o1->getCollisionObject() ) || is_ignored( o2->getCollisionObject() ) )
		{
			return 1.f;
		}

		is_hit_ = true;

		return 1.f;
	}

	bool is_hit() const { return is_hit_; }
};

} // namespace

PlayerBehaviorSystem::PlayerBehaviorSystem( const ActiveObjectPhysics* physics, graphics::GraphicsManager* graphics_manager, core::sound::SoundManager* sound_manager )
	: physics_( physics )
	, graphics_manager_( graphics_manager )
	, sound_manager_( sound_manager )
{

}

PlayerBehaviorComponent* PlayerBehaviorSystem::find_player( core::ecs::EntityId id ) const
{
	core::ecs::Entity* entity = get_entity_manager()->find( id );

	return entity ? entity->get_component< PlayerBehaviorComponent >() : nullptr;
}

bool PlayerBehaviorSystem::find_self( core::ecs::EntityId id, Self& self ) const
{
	self.entity = get_entity_manager()->find( id );
	self.p = self.entity ? self.entity->get_component< PlayerBehaviorComponent >() : nullptr;

	if ( ! self.p )
	{
		return false;
	}

	self.object = EntityObject( self.entity );
	self.body = self.object.get_rigid_body();

	return self.object.is_valid();
}

void PlayerBehaviorSystem::play_sound( const Self& self, const char_t* name, bool loop, bool force ) const
{
	self.object.play_sound( sound_manager_, name, loop, force );
}

void PlayerBehaviorSystem::stop_sound( const char_t* name ) const
{
	EntityObject::stop_sound( sound_manager_, name );
}

void PlayerBehaviorSystem::fade_out_sound( const char_t* name ) const
{
	if ( core::Sound* sound = sound_manager_ ? sound_manager_->get_sound( name ) : nullptr )
	{
		sound->fade_out();
	}
}

void PlayerBehaviorSystem::restart( core::ecs::EntityId id )
{
	Self self;

	if ( ! find_self( id, self ) )
	{
		return;
	}

	PlayerBehaviorComponent& p = * self.p;

	if ( self.body )
	{
		object_body::set_angular_factor( * self.body, 0.f );
		object_body::set_friction( * self.body, 0.f );
		object_body::set_mass( * self.body, PlayerBehaviorComponent::PlayerMass );
	}

	p.is_on_footing = false;
	p.is_jumping = false;
	p.is_jumpable = false;
	p.is_clambering = false;
	p.is_located_on_die = false;
	p.is_located_on_safe = false;
	p.is_on_ladder = false;
	p.is_facing_to_block = false;
	p.can_clamber = false;
	p.can_peer_down = false;
	p.can_throw = false;

	p.is_flickering = false;

	p.step_count = 0;
	p.step_speed = PlayerBehaviorComponent::MaxWalkStepSpeed;

	set_action_mode( self, ActionMode::NONE );

	p.is_action_pre_finish = false;
	p.balloon_sequence_count = 0;
	p.balloon_sound_request = 0;

	p.pitch = 0.f;

	p.eye_height = PlayerBehaviorComponent::DefaultEyeHeight;
	p.eye_depth = 0.f;

	p.has_medal = false;

	for ( int n = 0; n < static_cast< int >( ItemType::MAX ); n++ )
	{
		p.item_count[ n ] = 0;
	}

	p.item_count[ static_cast< int >( ItemType::SCOPE ) ] = 1;

	p.selected_item_type = ItemType::NONE;

	p.balloon.reset();
	p.stone_list.clear();

	p.hp = 1;

	p.last_footing_height = self.location().y();
}

void PlayerBehaviorSystem::update_players( float_t delta_time, float_t scene_elapsed_time )
{
	id_list_.clear();

	for ( const auto& i : get_component_list() )
	{
		id_list_.push_back( i.first );
	}

	std::sort( id_list_.begin(), id_list_.end() );

	for ( const core::ecs::EntityId id : id_list_ )
	{
		update_player( id, delta_time, scene_elapsed_time );
	}
}

void PlayerBehaviorSystem::update_player( core::ecs::EntityId id, float_t delta_time, float_t scene_elapsed_time )
{
	Self self;

	if ( ! find_self( id, self ) || ! self.body )
	{
		return;
	}

	self.delta_time = delta_time;
	self.scene_elapsed_time = scene_elapsed_time;

	PlayerBehaviorComponent& p = * self.p;

	self.body->setActivationState( true );

	update_on_footing( self );
	update_jumpable( self );
	update_jumping( self );
	update_located_to_die( self );
	update_facing_to_block( self );
	update_can_clamber( self );
	update_can_peer_down( self );
	update_can_throw( self );

	p.action_timer += delta_time;

	if ( p.action_mode == ActionMode::BALLOON )
	{
		const Vector v = self.velocity();
		self.object.set_velocity( Vector( v.x(), std::min( PlayerBehaviorComponent::BalloonMaxVelocity, math::chase( v.y(), PlayerBehaviorComponent::BalloonMaxVelocity, v.y() < 0.f ? PlayerBehaviorComponent::BalloonChaseSpeedDown : PlayerBehaviorComponent::BalloonChaseSpeedUp ) ), v.z() ) );

		if ( self.location().y() - p.action_base_position.y() >= PlayerBehaviorComponent::BalloonActionLength )
		{
			play_sound( self, "balloon-burst" );
			set_action_mode( self, ActionMode::NONE );
			p.is_jumping = true;
			p.is_action_pre_finish = false;
		}
		else if ( self.location().y() - p.action_base_position.y() >= PlayerBehaviorComponent::BalloonActionLength * PlayerBehaviorComponent::BalloonPreFinishRatio )
		{
			p.is_action_pre_finish = true;
		}
	}
	else if ( p.action_mode == ActionMode::ROCKET )
	{
		// ロケット
		if ( ( self.location() - p.action_base_position ).length() >= PlayerBehaviorComponent::RocketActionLength || p.action_timer >= PlayerBehaviorComponent::RocketTimeout )
		{
			finish_rocketing( self );
		}
		else if ( ( self.location() - p.action_base_position ).length() >= PlayerBehaviorComponent::RocketActionLength * PlayerBehaviorComponent::RocketPreFinishRatio )
		{
			p.is_action_pre_finish = true;
		}
	}

	if ( p.is_flickering )
	{
		const auto* flicker = self.entity->get_component< FlickerComponent >();
		self.object.update_velocity_by_flicker( Vector( self.location().x(), p.action_base_position.y(), self.location().z() ), flicker ? flicker->scale : 1.f, scene_elapsed_time );
	}

	if ( ! p.can_peer_down )
	{
		p.eye_depth *= PlayerBehaviorComponent::EyeDepthDecay;
		p.eye_depth = math::clamp( p.eye_depth, 0.f, 1.f );
	}

	// 線の色（モデルはプレイヤーだけのもの。描画が無ければ変えない）
	const auto* render = self.entity->get_component< ObjectRenderComponent >();
	graphics::Model* model = render && graphics_manager_ ? graphics_manager_->resolve_model( render->model ) : nullptr;
	graphics::Line* line = model ? model->get_line() : nullptr;

	if ( p.is_located_on_safe )
	{
		if ( line ) { line->set_color( core::Color( 0.25f, 0.25f, 1.f, 0.f ) ); }
	}
	else if ( p.is_located_on_die )
	{
		if ( line ) { line->set_color( core::Color( 1.f, 0.f, 0.f, 0.f ) ); }

		if ( ! self.object.is_dead() && ! p.is_on_footing && ! p.is_on_ladder && self.body->getLinearVelocity().y() < PlayerBehaviorComponent::FallSoundVelocity && self.location().y() > 10.f && self.location().y() < 30.f && p.action_mode == ActionMode::NONE )
		{
			play_sound( self, "fall", false, false );
		}
	}
	else
	{
		if ( line ) { line->set_color( core::Color( 0.f, 0.f, 0.f, -0.25f ) ); }
	}

	if ( p.is_on_footing )
	{
		if ( p.is_located_on_die )
		{
			kill( self );
		}
		else
		{
			// 通常着地
			if ( self.velocity().y() < -PlayerBehaviorComponent::LandingVelocityThreshold )
			{
				stop_sound( "fall" );
				play_sound( self, "land", false, false );

				// バウンドしないようにする
				const Vector v = self.velocity();
				self.object.set_velocity( Vector( v.x(), 0.f, v.z() ) );
			}

			p.last_footing_height = self.location().y();
		}

		if ( p.action_mode != ActionMode::BALLOON )
		{
			p.balloon_sequence_count = 0;
		}
	}

	if ( ! p.is_located_on_die || p.is_on_ladder )
	{
		stop_sound( "fall" );
	}

	if ( self.object.is_dead() )
	{
		p.eye_height *= PlayerBehaviorComponent::DeadEyeDecay;
		p.eye_height = std::max( p.eye_height, PlayerBehaviorComponent::DeadEyeMinHeight );
	}

	p.uncontrollable_timer = math::chase< float_t >( p.uncontrollable_timer, 0.f, delta_time );

	limit_velocity( self );

	update_gravity( self );

	// 当たり判定のためにリセット
	p.is_on_ladder = false;
}

void PlayerBehaviorSystem::limit_velocity( Self& self )
{
	const PlayerBehaviorComponent& p = * self.p;

	// 横（x・z）の速さを ±20 に収める（以前の ActiveObject::limit_velocity）
	auto limit_max_speed = [ & ]
	{
		Vector v( self.body->getLinearVelocity() );

		v.set_x( math::clamp( v.x(), -PlayerBehaviorComponent::MaxSpeed, PlayerBehaviorComponent::MaxSpeed ) );
		v.set_z( math::clamp( v.z(), -PlayerBehaviorComponent::MaxSpeed, PlayerBehaviorComponent::MaxSpeed ) );

		self.object.set_velocity( v );
	};

	if ( p.is_uncontrollable() )
	{
		limit_max_speed();
		return;
	}

	Vector v = self.velocity();

	if ( p.is_rocketing() )
	{
	}
	else if ( p.is_jumping )
	{
		v.set_x( v.x() * PlayerBehaviorComponent::VelocityDampingJumping );
		v.set_z( v.z() * PlayerBehaviorComponent::VelocityDampingJumping );
	}
	else
	{
		v.set_x( v.x() * PlayerBehaviorComponent::VelocityDampingNormal );
		v.set_z( v.z() * PlayerBehaviorComponent::VelocityDampingNormal );

		if ( p.is_on_ladder )
		{
			v.set_y( v.y() * PlayerBehaviorComponent::VelocityDampingLadder );
		}
	}

	if ( ! p.is_rocketing() )
	{
		Vector xz_v( v.x(), 0.f, v.z() );

		if ( xz_v.length() > PlayerBehaviorComponent::MaxRunVelocity )
		{
			xz_v /= xz_v.length() / PlayerBehaviorComponent::MaxRunVelocity;
			v.set_x( xz_v.x() );
			v.set_z( xz_v.z() );
		}
	}

	self.object.set_velocity( v );

	limit_max_speed();
}

/// 現在ジャンプ可能かどうかのフラグを更新する
void PlayerBehaviorSystem::update_jumpable( Self& self )
{
	PlayerBehaviorComponent& p = * self.p;

	p.is_jumpable = false;

	if ( p.action_mode == ActionMode::BALLOON || p.action_mode == ActionMode::ROCKET )
	{
		p.is_jumpable = true;
		return;
	}

	// ジャンプ直後と着地直後はジャンプできない
	if ( std::abs( self.velocity().y() ) > PlayerBehaviorComponent::JumpVelocityThreshold )
	{
		return;
	}

	p.is_jumpable = p.is_on_footing || ( p.is_on_ladder && ! p.is_facing_to_block );
}

void PlayerBehaviorSystem::update_on_footing( Self& self )
{
	const btVector3& c = self.body->getCenterOfMassPosition();
	const float_t ray_length = PlayerBehaviorComponent::CollisionHeight * 0.5f + 0.05f;

	const float_t margin = 0.05f;
	const Vector front = self.front() * ( PlayerBehaviorComponent::CollisionDepth * 0.5f - margin );
	const Vector right = self.right() * ( PlayerBehaviorComponent::CollisionWidth * 0.5f - margin );
	const Vector center = Vector( c.x(), c.y(), c.z() );

	// 中心 + 四隅 + 四点 の設置を調べる
	self.p->is_on_footing =
		check_on_footing( self, center, ray_length ) ||

		check_on_footing( self, center - right - front, ray_length ) ||
		check_on_footing( self, center + right - front, ray_length ) ||
		check_on_footing( self, center - right + front, ray_length ) ||
		check_on_footing( self, center + right + front, ray_length ) ||

		check_on_footing( self, center - right, ray_length ) ||
		check_on_footing( self, center + right, ray_length ) ||
		check_on_footing( self, center - front, ray_length ) ||
		check_on_footing( self, center + front, ray_length );
}

/// 現在ジャンプ中かどうかのフラグを更新する
void PlayerBehaviorSystem::update_jumping( Self& self )
{
	PlayerBehaviorComponent& p = * self.p;

	if ( p.is_jumping && self.velocity().y() < 0.f && ( p.is_on_footing || p.is_on_ladder ) )
	{
		p.is_jumping = false;
	}
}

/// このまま落下すると死亡するかどうかの状態を更新する
void PlayerBehaviorSystem::update_located_to_die( Self& self )
{
	PlayerBehaviorComponent& p = * self.p;

	const btVector3& c = self.body->getCenterOfMassPosition();

	const float_t margin = 0.05f;
	const Vector front = self.front() * ( PlayerBehaviorComponent::CollisionDepth * 0.5f - margin );
	const Vector right = self.right() * ( PlayerBehaviorComponent::CollisionWidth * 0.5f - margin );
	const Vector center = Vector( c.x(), c.y(), c.z() );

	p.is_located_on_safe = false;

	auto is_safe_below = [ & ] ( const Vector& from )
	{
		return p.last_footing_height - get_footing_height( self, from, true ) < PlayerBehaviorComponent::HeightToDie;
	};

	// 中心 + 四隅 + 四点 の設置を調べる（以前と同じく、安全な所が見つかったら残りは調べない）
	p.is_located_on_die = ! (
		is_safe_below( center ) ||

		is_safe_below( center - right - front ) ||
		is_safe_below( center + right - front ) ||
		is_safe_below( center - right + front ) ||
		is_safe_below( center + right + front ) ||

		is_safe_below( center - right ) ||
		is_safe_below( center + right ) ||
		is_safe_below( center - front ) ||
		is_safe_below( center + front ) );
}

bool PlayerBehaviorSystem::is_box_touching_block( const Self& self, const Vector& half, const Vector& offset_position ) const
{
	if ( ! physics_ )
	{
		return false;
	}

	btBoxShape shape( btVector3( half.x(), half.y(), half.z() ) );

	btTransform offset;
	offset.setIdentity();
	offset.setOrigin( btVector3( offset_position.x(), offset_position.y(), offset_position.z() ) );

	btCollisionObject collision_object;
	collision_object.setCollisionShape( & shape );
	collision_object.setWorldTransform( btTransform( self.object.get_transform() ) * offset );

	WithoutMeContactResultCallback callback( self.body, get_entity_manager() );
	physics_->contact_test( & collision_object, callback );

	return callback.is_hit();
}

/// 障害物に直面しているかのフラグを更新する
void PlayerBehaviorSystem::update_facing_to_block( Self& self )
{
	PlayerBehaviorComponent& p = * self.p;

	p.is_facing_to_block = false;

	const Vector half( 0.05f, 0.05f, 0.05f );
	const float_t half_y_with_margin = btBoxShape( btVector3( half.x(), half.y(), half.z() ) ).getHalfExtentsWithMargin().y();

	p.is_facing_to_block = is_box_touching_block( self, half, Vector( 0.f, p.is_clambering ? -0.1f : half_y_with_margin + 0.25f, PlayerBehaviorComponent::CollisionDepth * 0.5f ) );
}

/// よじ登りが可能かのフラグを更新する
void PlayerBehaviorSystem::update_can_clamber( Self& self )
{
	PlayerBehaviorComponent& p = * self.p;

	p.can_clamber = false;

	if ( ! p.is_facing_to_block )
	{
		return;
	}

	if ( p.last_footing_height - self.location().y() > PlayerBehaviorComponent::MaxHeightDropForClamber )
	{
		return;
	}

	const Vector half( PlayerBehaviorComponent::CollisionWidth * 0.5f, PlayerBehaviorComponent::CollisionHeight * 0.5f, PlayerBehaviorComponent::CollisionDepth * 0.5f );

	if ( ! is_box_touching_block( self, half, Vector( 0.f, PlayerBehaviorComponent::CollisionHeight * 0.5f + 0.1f, PlayerBehaviorComponent::CollisionDepth * 0.5f ) ) )
	{
		p.can_clamber = true;
		return;
	}

	p.can_clamber = ! is_box_touching_block( self, half, Vector( 0.f, PlayerBehaviorComponent::CollisionHeight * 0.5f + 1.5f, PlayerBehaviorComponent::CollisionDepth ) );
}

/// のぞき込みが可能かのフラグを更新する
void PlayerBehaviorSystem::update_can_peer_down( Self& self )
{
	PlayerBehaviorComponent& p = * self.p;

	if ( ! p.is_on_footing )
	{
		p.can_peer_down = false;
		return;
	}

	p.can_peer_down = ! is_box_touching_block( self, Vector( 0.25f, 0.25f, 0.25f ), Vector( 0.f, p.eye_height, PlayerBehaviorComponent::CollisionDepth * 0.5f + p.eye_depth ) );
}

/// 石投げが可能かのフラグを更新する
void PlayerBehaviorSystem::update_can_throw( Self& self )
{
	PlayerBehaviorComponent& p = * self.p;

	p.can_throw = ! is_box_touching_block( self, Vector( 0.25f, 0.25f, 0.25f ), Vector( 0.f, p.eye_height, PlayerBehaviorComponent::CollisionDepth ) );
}

/// 移動速度を更新する
void PlayerBehaviorSystem::update_step_speed( Self& self )
{
	PlayerBehaviorComponent& p = * self.p;

	if ( p.action_mode == ActionMode::BALLOON )
	{
		p.step_speed = math::chase( p.step_speed, PlayerBehaviorComponent::MaxWalkStepSpeed, PlayerBehaviorComponent::StepSpeedChaseRate );
	}
	else if ( p.step_count <= 0 )
	{
		// stop
		p.step_speed *= PlayerBehaviorComponent::StepSpeedStopDecay;
	}
	else if ( p.step_count <= PlayerBehaviorComponent::StepCountToRun || ! can_running( self ) )
	{
		// slow walk
		p.step_speed = math::chase( p.step_speed, PlayerBehaviorComponent::MaxWalkStepSpeed, PlayerBehaviorComponent::StepSpeedChaseRate );
	}
	else
	{
		// run
		p.step_speed = math::chase( p.step_speed, PlayerBehaviorComponent::MaxRunStepSpeed, PlayerBehaviorComponent::RunSpeedChaseRate );
	}

	if ( ! p.is_on_footing )
	{
		stop_sound( "walk" );
		stop_sound( "run" );
	}
	else if ( p.is_running() )
	{
		stop_sound( "walk" );
		play_sound( self, "run", true, false );
	}
	else if ( p.is_walking() )
	{
		stop_sound( "run" );
		play_sound( self, "walk", true, false );
	}
	else
	{
		stop_sound( "walk" );
		stop_sound( "run" );
	}

	if ( p.step_speed >= PlayerBehaviorComponent::MaxRunStepSpeed && ! p.is_jumping && ! is_falling( self ) )
	{
		play_sound( self, "short-breath", true, false );
	}
	else
	{
		fade_out_sound( "short-breath" );
	}
}

/// 走ることができるかを調べる
bool PlayerBehaviorSystem::can_running( const Self& self ) const
{
	return self.velocity().length() > PlayerBehaviorComponent::MinVelocityForRun && self.p->item_count[ static_cast< int >( ItemType::STONE ) ] < PlayerBehaviorComponent::MaxStonesForRunning;
}

/// 重力を更新する
void PlayerBehaviorSystem::update_gravity( Self& self )
{
	const PlayerBehaviorComponent& p = * self.p;

	if ( p.action_mode == ActionMode::BALLOON || p.action_mode == ActionMode::ROCKET || ( p.is_on_ladder && ! p.is_jumping ) )
	{
		self.body->setGravity( Vector::Zero );
	}
	else
	{
		const auto* state = self.entity->get_component< SimpleObjectComponent >();
		self.body->setGravity( state ? state->default_gravity : Vector::Zero );
	}
}

/**
 * 足元の近くに足場があるか
 *
 * @param from レイを飛ばし始める原点
 * @param ray_length レイの長さ
 */
bool PlayerBehaviorSystem::check_on_footing( const Self& self, const Vector& from, float_t ray_length, bool include_soft_footing ) const
{
	if ( ! physics_ )
	{
		return false;
	}

	Vector to = from + Vector( 0, -ray_length, 0 );

	ClosestNotMe ray_callback( from, to, self.body, include_soft_footing, get_entity_manager() );
	ray_callback.m_closestHitFraction = 1.0;

	physics_->ray_test( from, to, ray_callback );

	return ray_callback.hasHit();
}

/**
 * 指定した座標から地面までの間で、一番高い足場の高さを取得する
 *
 * @param from
 * @param include_soft_footing 風船などのやわらかいものを足場に含むフラグ
 */
float_t PlayerBehaviorSystem::get_footing_height( Self& self, const Vector& from, bool include_soft_footing ) const
{
	if ( ! physics_ )
	{
		return 0.f;
	}

	Vector to = from;
	to.set_y( 0.f );

	ClosestNotMe ray_callback( from, to, self.body, include_soft_footing, get_entity_manager() );
	ray_callback.m_closestHitFraction = 1.0;

	physics_->ray_test( from, to, ray_callback );

	// 当たった物の性質は、剛体の Entity の番号のタグから引く
	if ( const auto* traits = CollisionEventSystem::find_traits( get_entity_manager(), ray_callback.m_collisionObject ) )
	{
		if ( traits->is_safe_footing )
		{
			self.p->is_located_on_safe = true;
		}
	}

	return ( 1.f - ray_callback.m_closestHitFraction ) * from.y();
}

bool PlayerBehaviorSystem::is_falling( const Self& self ) const
{
	return self.body && self.body->getLinearVelocity().y() < PlayerBehaviorComponent::FallingVelocityThreshold;
}

void PlayerBehaviorSystem::set_action_mode( Self& self, ActionMode action_mode )
{
	PlayerBehaviorComponent& p = * self.p;

	if ( action_mode == p.action_mode )
	{
		return;
	}

	p.action_mode = action_mode;
	p.action_timer = 0.f;

	stop_sound( "fall" );
}

void PlayerBehaviorSystem::finish_rocketing( Self& self )
{
	stop_sound( "rocket" );
	play_sound( self, "rocket-burst" );
	set_action_mode( self, ActionMode::NONE );
	self.p->is_jumping = true;
	self.p->is_action_pre_finish = false;
}

void PlayerBehaviorSystem::set_eye_depth( Self& self, float_t d )
{
	if ( ! self.p->is_on_footing )
	{
		return;
	}

	if ( self.location().y() < PlayerBehaviorComponent::MinHeightForPeer )
	{
		return;
	}

	self.p->eye_depth = math::clamp( d, 0.f, 1.f );
}

void PlayerBehaviorSystem::kill( Self& self )
{
	if ( self.object.is_dead() )
	{
		return;
	}

	// 以前の ActiveObject::kill（当たりを起こさず、描かない）。EntityObject::kill はプレイヤーならここへ来るので呼ばない
	if ( auto* traits = self.entity->get_component< CollisionTraitsComponent >() )
	{
		traits->is_dead = true;
	}

	self.object.set_visible( false, false );

	stop_sound( "fall" );
	stop_sound( "walk" );
	stop_sound( "run" );

	play_sound( self, "dead" );

	if ( self.body )
	{
		self.body->setActivationState( true );
		object_body::set_angular_factor( * self.body, 1.f );
		object_body::set_friction( * self.body, 1.f );
	}
}

// ---- 操作 ----

void PlayerBehaviorSystem::step( core::ecs::EntityId id, float_t s )
{
	Self self;

	if ( ! find_self( id, self ) || ! self.body )
	{
		return;
	}

	PlayerBehaviorComponent& p = * self.p;

	if ( p.is_uncontrollable() || p.is_rocketing() )
	{
		return;
	}

	p.step_count++;
	update_step_speed( self );

	const Vector front = self.front() * s;
	self.object.set_velocity( self.velocity() + Vector( front.x() * p.step_speed, 0.f, front.z() * p.step_speed ) );
}

void PlayerBehaviorSystem::side_step( core::ecs::EntityId id, float_t s )
{
	Self self;

	if ( ! find_self( id, self ) || ! self.body )
	{
		return;
	}

	PlayerBehaviorComponent& p = * self.p;

	if ( p.is_uncontrollable() || p.is_rocketing() )
	{
		return;
	}

	const Vector right = self.right() * s;
	self.object.set_velocity( self.velocity() + Vector( right.x() * PlayerBehaviorComponent::SideStepSpeed, 0.f, right.z() * PlayerBehaviorComponent::SideStepSpeed ) );
}

void PlayerBehaviorSystem::ladder_step( core::ecs::EntityId id, float_t s )
{
	Self self;

	if ( ! find_self( id, self ) || ! self.body )
	{
		return;
	}

	PlayerBehaviorComponent& p = * self.p;

	if ( p.is_uncontrollable() || p.is_jumping )
	{
		return;
	}

	Vector v = self.velocity() + Vector( 0.f, p.pitch * s, 0.f );
	v.set_y( math::clamp( v.y(), -PlayerBehaviorComponent::LadderStepSpeed, PlayerBehaviorComponent::LadderStepSpeed ) );

	self.object.set_velocity( v );

	play_sound( self, "ladder-step", true, false );
}

void PlayerBehaviorSystem::stop( core::ecs::EntityId id )
{
	Self self;

	if ( ! find_self( id, self ) || ! self.body )
	{
		return;
	}

	self.p->step_count = 0;
	update_step_speed( self );
}

void PlayerBehaviorSystem::stop_ladder_step( core::ecs::EntityId )
{
	stop_sound( "ladder-step" );
}

void PlayerBehaviorSystem::jump( core::ecs::EntityId id )
{
	Self self;

	if ( ! find_self( id, self ) || ! self.body )
	{
		return;
	}

	PlayerBehaviorComponent& p = * self.p;

	if ( ! p.is_jumpable )
	{
		return;
	}

	if ( p.is_on_ladder )
	{
		self.object.set_velocity( self.velocity() + Vector( 0.f, PlayerBehaviorComponent::JumpVelocity, 0.f ) + self.front() * PlayerBehaviorComponent::JumpVelocity );
	}
	else
	{
		self.object.set_velocity( self.velocity() + Vector( 0.f, PlayerBehaviorComponent::JumpVelocity, 0.f ) );
	}

	p.is_jumping = true;

	set_action_mode( self, ActionMode::NONE );

	stop_sound( "short-breath" );

	if ( p.is_running() )
	{
		play_sound( self, "short-breath-jump" );
	}
	else
	{
		play_sound( self, "jump" );
	}
}

void PlayerBehaviorSystem::clamber( core::ecs::EntityId id )
{
	Self self;

	if ( ! find_self( id, self ) || ! self.body )
	{
		return;
	}

	PlayerBehaviorComponent& p = * self.p;

	if ( p.can_clamber )
	{
		self.object.set_velocity( Vector( 0.f, PlayerBehaviorComponent::ClamberVelocity, 0.f ) + self.front() * PlayerBehaviorComponent::ClamberVelocity );

		if ( ! p.is_clambering )
		{
			p.is_clambering = true;

			play_sound( self, "clamber", false, false );
		}
	}
	else
	{
		p.is_clambering = false;
	}
}

void PlayerBehaviorSystem::stop_clamber( core::ecs::EntityId id )
{
	if ( auto* p = find_player( id ) )
	{
		p->is_clambering = false;
	}
}

void PlayerBehaviorSystem::add_direction_degree( core::ecs::EntityId id, float_t d )
{
	Self self;

	if ( ! find_self( id, self ) )
	{
		return;
	}

	set_direction_degree( id, self.object.get_direction_degree() + d );
}

void PlayerBehaviorSystem::set_direction_degree( core::ecs::EntityId id, float_t d, bool force )
{
	Self self;

	if ( ! find_self( id, self ) )
	{
		return;
	}

	if ( ! force && self.p->is_rocketing() )
	{
		return;
	}

	self.object.set_direction_degree( d );
}

void PlayerBehaviorSystem::set_pitch( core::ecs::EntityId id, float_t pitch )
{
	if ( auto* p = find_player( id ) )
	{
		p->pitch = pitch;
	}
}

void PlayerBehaviorSystem::add_eye_depth( core::ecs::EntityId id, float_t d )
{
	Self self;

	if ( ! find_self( id, self ) )
	{
		return;
	}

	if ( d > 0.f && ! self.p->can_peer_down )
	{
		return;
	}

	set_eye_depth( self, self.p->eye_depth + d );
}

void PlayerBehaviorSystem::rocket( core::ecs::EntityId id, const Vector& direction )
{
	Self self;

	if ( ! find_self( id, self ) || ! self.body )
	{
		return;
	}

	PlayerBehaviorComponent& p = * self.p;

	if ( p.get_item_count( ItemType::ROCKET ) <= 0 )
	{
		return;
	}

	p.is_jumping = true;
	p.action_timer = 0.f;

	set_action_mode( self, ActionMode::ROCKET );
	p.action_base_position = self.location();

	self.object.set_velocity( direction * PlayerBehaviorComponent::RocketInitialVelocity );

	p.item_count[ static_cast< int >( ItemType::ROCKET ) ]--;

	if ( p.item_count[ static_cast< int >( ItemType::ROCKET ) ] <= 0 )
	{
		p.selected_item_type = ItemType::NONE;
	}

	play_sound( self, "rocket" );
}

void PlayerBehaviorSystem::throw_stone( core::ecs::EntityId id, const Vector& direction )
{
	Self self;

	if ( ! find_self( id, self ) )
	{
		return;
	}

	PlayerBehaviorComponent& p = * self.p;

	if ( p.get_item_count( ItemType::STONE ) <= 0 )
	{
		return;
	}

	if ( ! p.can_throw )
	{
		return;
	}

	GameObjectRef stone = p.stone_list.back();
	p.stone_list.pop_back();
	p.item_count[ static_cast< int >( ItemType::STONE ) ]--;

	if ( p.item_count[ static_cast< int >( ItemType::STONE ) ] <= 0 )
	{
		p.selected_item_type = ItemType::NONE;
	}

	stone.restart();
	stone.set_location( self.location() + ( self.front() * PlayerBehaviorComponent::CollisionDepth ) + Vector( 0.f, p.eye_height, 0.f ) );
	stone.set_velocity( direction * PlayerBehaviorComponent::StoneThrowSpeed );

	play_sound( self, "stone-throw" );
}

void PlayerBehaviorSystem::select_prev_item( core::ecs::EntityId id )
{
	Self self;

	if ( ! find_self( id, self ) )
	{
		return;
	}

	PlayerBehaviorComponent& p = * self.p;

	for ( int type = static_cast< int >( p.selected_item_type ) - 1; type >= static_cast< int >( ItemType::NONE ); type-- )
	{
		if ( type == static_cast< int >( ItemType::NONE ) || p.item_count[ type ] )
		{
			p.selected_item_type = static_cast< ItemType >( type );
			play_sound( self, "click" );
			break;
		}
	}
}

void PlayerBehaviorSystem::select_next_item( core::ecs::EntityId id )
{
	Self self;

	if ( ! find_self( id, self ) )
	{
		return;
	}

	PlayerBehaviorComponent& p = * self.p;

	for ( int type = static_cast< int >( p.selected_item_type ) + 1; type < static_cast< int >( ItemType::MAX ); type++ )
	{
		if ( p.item_count[ type ] )
		{
			p.selected_item_type = static_cast< ItemType >( type );
			play_sound( self, "click" );
			break;
		}
	}
}

/// 望遠鏡モードを切り替える
void PlayerBehaviorSystem::switch_scope_mode( core::ecs::EntityId id )
{
	Self self;

	if ( ! find_self( id, self ) )
	{
		return;
	}

	PlayerBehaviorComponent& p = * self.p;

	if ( p.action_mode == ActionMode::SCOPE )
	{
		set_action_mode( self, ActionMode::NONE );
		p.selected_item_type = ItemType::NONE;
	}
	else
	{
		p.action_mode = ActionMode::SCOPE;		// 以前と同じく、経過時間は戻さず落ちる音も止めない
	}
}

void PlayerBehaviorSystem::start_flickering( core::ecs::EntityId id )
{
	Self self;

	if ( ! find_self( id, self ) )
	{
		return;
	}

	self.p->is_flickering = true;
	self.p->action_base_position = self.location();
}

void PlayerBehaviorSystem::kill( core::ecs::EntityId id )
{
	Self self;

	if ( find_self( id, self ) )
	{
		kill( self );
	}
}

// ---- 当たり ----

bool PlayerBehaviorSystem::on_touch_balloon( core::ecs::EntityId id, core::ecs::EntityId balloon )
{
	Self self;

	if ( ! find_self( id, self ) )
	{
		return false;
	}

	PlayerBehaviorComponent& p = * self.p;

	if ( p.is_flickering )
	{
		return false;
	}

	p.balloon = balloon;

	p.is_jumping = false;

	set_action_mode( self, ActionMode::BALLOON );
	p.action_base_position = self.location();

	p.last_footing_height = self.location().y();

	p.is_flickering = false;

	p.balloon_sequence_count += 1;
	p.balloon_sound_request = p.balloon_sequence_count;

	return true;
}

bool PlayerBehaviorSystem::on_touch_stone( core::ecs::EntityId id, const GameObjectRef& stone )
{
	Self self;

	if ( ! find_self( id, self ) )
	{
		return false;
	}

	PlayerBehaviorComponent& p = * self.p;

	p.item_count[ static_cast< int >( ItemType::STONE ) ]++;
	p.selected_item_type = ItemType::STONE;

	p.stone_list.push_back( stone );

	play_sound( self, "stone-get" );

	return true;
}

bool PlayerBehaviorSystem::on_touch_pickup( core::ecs::EntityId id, PickupKind kind )
{
	Self self;

	if ( ! find_self( id, self ) )
	{
		return false;
	}

	PlayerBehaviorComponent& p = * self.p;

	switch ( kind )
	{
	case PickupKind::MEDAL:
		p.has_medal = true;
		play_sound( self, "medal-get" );
		break;

	case PickupKind::ROCKET:
		p.item_count[ static_cast< int >( ItemType::ROCKET ) ]++;
		p.selected_item_type = ItemType::ROCKET;
		play_sound( self, "rocket-get" );
		break;

	case PickupKind::UMBRELLA:
		// 以前から、傘を拾ってもプレイヤーは何もしない（傘は消える）
		break;

	default:
		break;
	}

	return true;
}

bool PlayerBehaviorSystem::on_touch_robot( core::ecs::EntityId id, bool is_chasing )
{
	Self self;

	if ( ! find_self( id, self ) )
	{
		return false;
	}

	if ( ! is_chasing )
	{
		return true;
	}

	PlayerBehaviorComponent& p = * self.p;

	if ( ! p.is_uncontrollable() )
	{
		p.hp--;
	}

	if ( p.hp <= 0 )
	{
		kill( self );

		return true;
	}

	// 押し返される（以前の Player::damage）
	Vector v = -self.front() * PlayerBehaviorComponent::KnockbackSpeed;
	v.set_y( PlayerBehaviorComponent::KnockbackYVelocity );

	p.uncontrollable_timer = PlayerBehaviorComponent::UncontrollableDuration;

	self.object.set_velocity( v );

	p.is_jumping = true;

	play_sound( self, "damage-1" );

	return true;
}

bool PlayerBehaviorSystem::on_touch_ladder( core::ecs::EntityId id )
{
	Self self;

	if ( ! find_self( id, self ) )
	{
		return false;
	}

	PlayerBehaviorComponent& p = * self.p;

	p.is_on_ladder = true;

	if ( p.is_rocketing() )
	{
		finish_rocketing( self );
	}

	p.last_footing_height = self.location().y();

	return true;
}

void PlayerBehaviorSystem::on_touch_static_object( core::ecs::EntityId id )
{
	Self self;

	if ( find_self( id, self ) && self.p->is_rocketing() )
	{
		finish_rocketing( self );
	}
}

// ---- 問い合わせ ----

bool PlayerBehaviorSystem::is_falling( core::ecs::EntityId id ) const
{
	Self self;

	return find_self( id, self ) && is_falling( self );
}

bool PlayerBehaviorSystem::is_falling_to_die( core::ecs::EntityId id ) const
{
	Self self;

	if ( ! find_self( id, self ) )
	{
		return false;
	}

	const PlayerBehaviorComponent& p = * self.p;

	return is_falling( self ) && ! p.is_on_ladder && ! p.is_rocketing() && p.is_located_on_die;
}

bool PlayerBehaviorSystem::is_dead( core::ecs::EntityId id ) const
{
	Self self;

	return find_self( id, self ) && self.object.is_dead();
}

Vector PlayerBehaviorSystem::get_balloon_hold_location( core::ecs::Entity& player )
{
	const EntityObject object( & player );

	if ( ! object.is_valid() )
	{
		return Vector::Zero;
	}

	return object.get_location() + object.get_front() * 0.5f - object.get_right() * 0.25f + Vector( 0, 1.75f, 0 );
}

} // namespace blue_sky::ecs
