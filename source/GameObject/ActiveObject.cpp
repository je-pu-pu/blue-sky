#include "ActiveObject.h"
#include <blue_sky/ActiveObjectPhysics.h>
#include <blue_sky/GameMain.h>

#include <blue_sky/ConstantBuffers.h>

#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/graphics/Model.h>
#include <blue_sky/graphics/Line.h>
#include <blue_sky/ecs/component/ObjectRenderComponent.h>
#include <blue_sky/ecs/component/AnimationComponent.h>
#include <blue_sky/ecs/component/CollisionTraitsComponent.h>
#include <blue_sky/ecs/component/ActiveObjectLinkComponent.h>
#include <blue_sky/ecs/component/PhysicsBodyComponent.h>
#include <blue_sky/DebugLog.h>
#include <blue_sky/GameObjectRef.h>

#include <core/physics/EntityTag.h>

#include <core/ecs/EntityManager.h>
#include <core/ecs/component/TransformComponent.h>

#include <core/math/Quaternion.h>
#include <core/sound/Sound.h>
#include <core/animation/AnimationPlayer.h>

#include <core/graphics/Shader.h>

#include <common/math.h>

#include <sstream>

namespace blue_sky
{

ActiveObject::ActiveObject()
	: front_( Vector::Forward )
	, right_( Vector::Right )

	, start_location_( Vector::Zero )
	, start_rotation_( Vector::Zero )
	, start_direction_degree_( 0 )

	, is_dead_( false )
	, flicker_scale_( 1.f )

	, direction_degree_( 0 )

	, is_mesh_visible_( true )
	, is_line_visible_( true )
{
	
}

ActiveObject::ActiveObject( const ActiveObject& o )
	: GameObject( o )
	, front_( o.front_ )
	, right_( o.right_ )

	, start_location_( o.start_location_ )
	, start_rotation_( o.start_rotation_ )
	, start_direction_degree_( o.start_direction_degree_ )

	, pending_model_( const_cast< Model* >( o.get_model() ) )		// Entity は作らない（ActiveObjectManager::clone_object が加えるときに作る）。モデルは共有する
	, is_dead_( o.is_dead_ )
	, flicker_scale_( o.flicker_scale_ )

	, direction_degree_( o.direction_degree_ )

	, is_mesh_visible_( o.is_mesh_visible_ )
	, is_line_visible_( o.is_line_visible_ )
{

}

ActiveObject::~ActiveObject()
{
	detach_entity();
}

GameObjectRef ActiveObject::get_ref() const
{
	if ( ! has_entity_ )
	{
		return GameObjectRef();
	}

	return GameObjectRef( entity_manager_, graphics_manager_, entity_id_ );
}

/**
 * 描画の部品（無ければ nullptr。Entity を作る前・EntityManager が片付けた後）
 *
 * 部品のポインタは持ち続けない（EntityManager がシーンの切り替えで片付けると無効になる）。使うたびに Entity から引く
 */
ecs::ObjectRenderComponent* ActiveObject::get_render_component() const
{
	if ( ! has_entity_ )
	{
		return nullptr;
	}

	core::ecs::Entity* entity = entity_manager_->find( entity_id_ );

	return entity ? entity->get_component< ecs::ObjectRenderComponent >() : nullptr;
}

/// アニメーションの部品（無ければ nullptr。アニメーションの無いモデル・Entity を作る前）
ecs::AnimationComponent* ActiveObject::get_animation_component() const
{
	if ( ! has_entity_ )
	{
		return nullptr;
	}

	core::ecs::Entity* entity = entity_manager_->find( entity_id_ );

	return entity ? entity->get_component< ecs::AnimationComponent >() : nullptr;
}

void ActiveObject::set_model( Model* m )
{
	auto* render = get_render_component();

	if ( ! render )
	{
		// Entity を作る前（ActiveObjectManager に加える前）は預かっておき、attach_entity で部品へ移す
		pending_model_ = m;
		return;
	}

	render->model = graphics_manager_->get_model_handle( m );

	if ( m && render->model.is_null() )
	{
		// GraphicsManager が管理していないモデルはハンドルにできず、描けない
		debug_log( "[Object] GraphicsManager が管理していないモデルは使えません（描きません）" );
	}

	setup_animation_player();
}

ActiveObject::Model* ActiveObject::get_model()
{
	return const_cast< Model* >( static_cast< const ActiveObject* >( this )->get_model() );
}

const ActiveObject::Model* ActiveObject::get_model() const
{
	if ( const auto* render = get_render_component() )
	{
		return graphics_manager_->resolve_model( render->model );	// 解放されたモデルなら nullptr
	}

	return pending_model_;
}

ActiveObject::AnimationPlayer* ActiveObject::get_animation_player()
{
	auto* animation = get_animation_component();
	return animation ? animation->player.get() : nullptr;
}

const ActiveObject::AnimationPlayer* ActiveObject::get_animation_player() const
{
	const auto* animation = get_animation_component();
	return animation ? animation->player.get() : nullptr;
}

/**
 * アニメーション再生をセットアップする
 *
 * モデルにアニメーションがあれば、アニメーションの部品とボーン行列の定数バッファを作る。既にあれば作り直さない（以前と同じ）
 */
void ActiveObject::setup_animation_player()
{
	// 部品がまだ無い（ActiveObjectManager に加える前）ときは、加えるときの attach_entity() で作る
	auto* render = get_render_component();

	if ( ! render || get_animation_player() )
	{
		return;
	}

	if ( get_model() && get_model()->get_skinning_animation_set() )
	{
		core::ecs::Entity* entity = entity_manager_->find( entity_id_ );

		auto* animation = entity->get_component< ecs::AnimationComponent >();

		if ( ! animation )
		{
			animation = entity->add_component< ecs::AnimationComponent >();
		}

		animation->player = std::make_unique< AnimationPlayer >( get_model()->get_skinning_animation_set() );
		render->bone_constant_buffer = graphics_manager_->create_constant_buffer( sizeof( core::BoneConstantBufferData ), core::BoneConstantBuffer::SLOT, core::graphics::ConstantBufferUsage::DYNAMIC );	// 毎フレーム書き換える
	}
}

void ActiveObject::attach_entity( core::ecs::EntityManager* entity_manager, graphics::GraphicsManager* graphics_manager )
{
	graphics_manager_ = graphics_manager;

	if ( has_entity_ && entity_manager_->is_alive( entity_id_ ) )
	{
		return;
	}

	if ( has_entity_ )
	{
		// Entity が外から消された（EntityManager が片付けた）。位置の正だった部品はもう無いので、写さずに戻す
		// （GameMain::setup_scene は EntityManager を片付ける前に物体を壊すので、本編では起きない）
		unbind_transform( false );
		has_entity_ = false;
	}

	entity_manager_ = entity_manager;

	core::ecs::Entity* entity = entity_manager_->create_entity();
	entity_id_ = entity->get_id();
	has_entity_ = true;

	// 位置の正を TransformComponent にする（以前は GameObject の Transform が正で、描く前に毎フレーム部品へ写していた）
	bind_transform( & entity->add_component< core::ecs::TransformComponent >()->transform );

	// 剛体（先に設定されていれば、預かっていた持ち主を部品へ移す。複製は元の物体と剛体を共有するので、持ち主を持たず剛体だけを指す）
	if ( get_rigid_body() )
	{
		auto* body = entity->add_component< ecs::PhysicsBodyComponent >();
		body->owner = std::move( pending_rigid_body_ );
		body->body = get_rigid_body();
		body->height_offset = get_height_offset();
	}

	// 目標地点・向きへ動かす窓口（MoveToTargetSystem が自分を位置・速度の窓口として呼ぶ）
	entity->add_component< ecs::TargetMoverComponent >()->mover = this;

	// 当たりの性質（種類ごとに決まった定数なので、ここで一度だけ写す）と、当たりのイベントから自分を引く印（互換の層）
	{
		auto* traits = entity->add_component< ecs::CollisionTraitsComponent >();
		traits->is_ghost = is_ghost();
		traits->is_hard = is_hard();
		traits->is_block = is_block();
		traits->is_safe_footing = is_safe_footing();
		traits->is_dead = is_dead_;

		entity->add_component< ecs::ActiveObjectLinkComponent >()->object = this;
	}

	// 剛体が先に設定されていれば、ここでタグを付ける（剛体が後なら set_rigid_body で付ける）
	apply_rigid_body_tag();

	auto* render = entity->add_component< ecs::ObjectRenderComponent >();

	// 毎フレーム書き換える（world・前フレームの world）ので DYNAMIC にする
	render->object_constant_buffer = graphics_manager_->create_constant_buffer( sizeof( ObjectConstantBufferData ), ObjectConstantBuffer::SLOT, core::graphics::ConstantBufferUsage::DYNAMIC );

	Model* model = pending_model_;
	pending_model_ = nullptr;

	ActiveObject::set_model( model );		// ハンドルにして部品へ移し、アニメーションを用意する（派生の set_model は預かったときに済んでいるので、もう一度は呼ばない）
	sync_render_component();
}

void ActiveObject::detach_entity()
{
	if ( ! has_entity_ )
	{
		return;
	}

	core::ecs::Entity* entity = entity_manager_->find( entity_id_ );

	if ( ! entity )
	{
		// もう消えている（EntityManager が片付けた）。部品は無いので、写さずに戻す
		unbind_transform( false );
		has_entity_ = false;
		return;
	}

	// 部品が消える前に、モデル・位置・剛体の持ち主を預かり直す（clone 元・もう一度加えるときのため。剛体は物体が壊れるときに消える）
	pending_model_ = get_model();
	unbind_transform();

	if ( auto* body = entity->get_component< ecs::PhysicsBodyComponent >() )
	{
		pending_rigid_body_ = std::move( body->owner );
	}

	// 剛体のタグを外す（以前は GameObject のデストラクタが userPointer を消していた。外した剛体は地面と同じ扱いになる）
	if ( is_rigid_body_owner_ && get_rigid_body() )
	{
		core::physics::clear_entity_tag( get_rigid_body() );
	}

	entity_manager_->destroy_entity( entity_id_ );
	has_entity_ = false;
}

void ActiveObject::set_rigid_body( core::physics::RigidBody&& rigid_body )
{
	btRigidBody* raw = rigid_body.get();

	set_rigid_body_view( raw );
	is_rigid_body_owner_ = raw != nullptr;

	core::ecs::Entity* entity = has_entity_ ? entity_manager_->find( entity_id_ ) : nullptr;

	if ( entity )
	{
		auto* body = entity->get_component< ecs::PhysicsBodyComponent >();

		if ( ! body )
		{
			body = entity->add_component< ecs::PhysicsBodyComponent >();
		}

		body->owner = std::move( rigid_body );		// 前の剛体があれば、ここで世界から外れて消える（以前は世界に残った）
		body->body = raw;
		body->height_offset = get_height_offset();
	}
	else
	{
		pending_rigid_body_ = std::move( rigid_body );
	}

	apply_rigid_body_tag();
}

void ActiveObject::apply_rigid_body_tag()
{
	if ( ! has_entity_ || ! is_rigid_body_owner_ || ! get_rigid_body() )
	{
		return;
	}

	core::physics::set_entity_tag( get_rigid_body(), entity_id_ );
}

void ActiveObject::sync_dead_to_collision_traits()
{
	if ( ! has_entity_ )
	{
		return;
	}

	core::ecs::Entity* entity = entity_manager_->find( entity_id_ );

	if ( auto* traits = entity ? entity->get_component< ecs::CollisionTraitsComponent >() : nullptr )
	{
		traits->is_dead = is_dead_;
	}
}

void ActiveObject::sync_render_component() const
{
	if ( ! has_entity_ )
	{
		return;
	}

	core::ecs::Entity* entity = entity_manager_->find( entity_id_ );

	if ( ! entity )
	{
		return;
	}

	// 位置は写さない（位置の正が TransformComponent になった。以前はここで GameObject の Transform を写していた）

	if ( auto* render = entity->get_component< ecs::ObjectRenderComponent >() )
	{
		render->is_visible = is_visible();
		render->is_mesh_visible = is_mesh_visible();
		render->is_line_visible = is_line_visible();
	}
}

/**
 * 目標地点に置いて止める（MoveToTargetSystem が着いたときに呼ぶ）
 *
 * 剛体の速度は ActiveObject を通して触る（剛体の持ち主は ActiveObjectPhysics のまま。set_velocity は同じ速度なら何もせず、変えるときは剛体を起こす）
 */
void ActiveObject::stop_at_target_location( const Vector& target_location )
{
	set_location( target_location );
	set_velocity( Vector::Zero );
}

/**
 * モデルとシェーダーを、このオブジェクトだけのものにする
 *
 * 複製したモデルも GraphicsManager が持つ（このオブジェクトは持たない）。部品のハンドルを複製のハンドルに差し替える
 * （replace_model で中身を替えると、同じモデルを共有するほかの物体まで変わってしまう）
 */
bool ActiveObject::make_model_unique()
{
	if ( ! get_render_component() || ! get_model() )
	{
		return false;
	}

	set_model( graphics_manager_->clone_model_with_shaders( get_model() ) );

	return true;
}

void ActiveObject::restart()
{
	is_dead_ = false;
	sync_dead_to_collision_traits();
	flicker_scale_ = 1.f;
	
	if ( get_rigid_body() )
	{
		get_transform().set_position( start_location_ );

		auto q = Quaternion::from_yaw_pitch_roll( math::degree_to_radian( start_rotation_.y() ), math::degree_to_radian( start_rotation_.x() ), math::degree_to_radian( start_rotation_.z() ) );
 		get_transform().set_rotation( q );

		get_rigid_body()->activate( true );
		get_rigid_body()->getMotionState()->setWorldTransform( get_transform() );
		
		get_rigid_body()->setWorldTransform( get_transform() );
		get_rigid_body()->setInterpolationWorldTransform( get_transform() );
		
		get_rigid_body()->setLinearVelocity( btVector3( 0.f, 0.f, 0.f ) );
		get_rigid_body()->setInterpolationLinearVelocity( btVector3( 0.f, 0.f, 0.f ) );
		
		get_rigid_body()->setAngularVelocity( btVector3( 0.f, 0.f, 0.f ) );
		get_rigid_body()->setInterpolationAngularVelocity( btVector3( 0.f, 0.f, 0.f ) );
		
		get_rigid_body()->setGravity( get_default_gravity() );

		get_rigid_body()->clearForces();

		set_direction_degree( start_direction_degree_ );
	}

	is_mesh_visible_ = true;
	is_line_visible_ = true;
}

void ActiveObject::limit_velocity()
{
	Vector v( get_rigid_body()->getLinearVelocity() );

	v.set_x( math::clamp( v.x(), -get_max_speed(), get_max_speed() ) );
	v.set_z( math::clamp( v.z(), -get_max_speed(), get_max_speed() ) );

	// debug
	// v.setY( math::clamp( v.y(), -0.1f, 0.1f ) );

	set_velocity( v );
}

void ActiveObject::set_start_location( float_t x, float_t y, float_t z )
{
	start_location_.set( x, y, z );
	get_transform().set_position( start_location_ );
}

void ActiveObject::set_start_rotation( float_t x, float_t y, float_t z )
{
	start_rotation_.set( x, y, z );

	Quaternion q{};
	q.set_yaw_pitch_roll( math::degree_to_radian( start_rotation_.y() ), math::degree_to_radian( start_rotation_.x() ), math::degree_to_radian( start_rotation_.z() ) );
	get_transform().set_rotation( q );
}

void ActiveObject::set_start_direction_degree( float_t d )
{
	start_direction_degree_ = d;
}

void ActiveObject::set_direction_degree( float_t d )
{
	{
		direction_degree_ = d;

		while ( direction_degree_ < 0.f ) direction_degree_ += 360.f;
		while ( direction_degree_ > 360.f ) direction_degree_ -= 360.f;
	}

	{
		Matrix m;
		m.set_rotation_xyz( 0, math::degree_to_radian( direction_degree_ ), 0 );

		front_ = Vector::Forward * m;
		right_ = Vector::Right * m;

		front_.normalize();
		right_.normalize();
	}

	{
		Quaternion q{};
		q.set_yaw_pitch_roll( math::degree_to_radian( direction_degree_ ), 0.f, 0.f );
		get_transform().set_rotation( q );

		commit_transform();
	}
}

/**
 * 方向を指定した場所の方向に近づける
 *
 * @param location 目的の場所
 * @param speed 速度
 */
void ActiveObject::chase_direction_to( const Vector& location, float_t speed )
{
	Vector relative_position = location - get_location();
	relative_position.set_y( 0 );

	float_t target_direction_degree = math::radian_to_degree( std::atan2( relative_position.x(), relative_position.z() ) );
	chase_direction_degree( target_direction_degree, speed );
}

/**
 * 方向を指定した目的の方向に近づける
 *
 * @param d 目的の方向
 * @param speed 速度
 */
void ActiveObject::chase_direction_degree( float_t d, float_t speed )
{
	while ( direction_degree_ < 0.f ) direction_degree_ += 360.f;
	while ( direction_degree_ > 360.f ) direction_degree_ -= 360.f;

	while ( d < 0.f ) d += 360.f;
	while ( d > 360.f ) d -= 360.f;

	float_t diff = d - direction_degree_;

	while ( diff < -180.f ) diff += 360.f;
	while ( diff >  180.f ) diff -= 360.f;

	set_direction_degree( math::chase( direction_degree_, direction_degree_ + diff, speed ) );
}

/**
 * オブジェクトを死亡させる
 *
 * オブジェクトは死亡すると is_visible(), is_mesh_visible() is_line_visible() 全てで false を返すようになるため、通常描画されなくなる。
 * ただし、ActiveObjectManager から削除される事はなく、update() は呼ばれ続ける。
 */
void ActiveObject::kill()
{
	is_dead_ = true;
	sync_dead_to_collision_traits();	// 当たり・レイの判定は部品の値を見る

	is_mesh_visible_ = false;
	is_line_visible_ = false;
}

void ActiveObject::play_animation( const char_t* name, bool force, bool loop )
{
	if ( ! get_animation_player() )
	{
		return;
	}

	get_animation_player()->play( name, force, loop );
}

/**
 * 固有のアクションを実行する
 * 
 * @param s
 */
void ActiveObject::action( const string_t& s )
{
	if ( s == "break_animation 1" && get_animation_player()	)
	{
		get_animation_player()->set_broken( true );
	}
	else if ( s == "break_animation 0" && get_animation_player() )
	{
		get_animation_player()->set_broken( false );
	}
	else
	{
		std::stringstream ss;
		ss << s;

		string_t command;
		ss >> command;

		if ( command == "set_visible" )
		{
			bool model = true, line = true;

			ss >> model >> line;

			set_mesh_visible( model );
			set_line_visible( line );
		}
	}
}

} // namespace blue_sky
