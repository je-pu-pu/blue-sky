#include "ActiveObject.h"
#include "ActiveObjectPhysics.h"
#include "GameMain.h"

#include <blue_sky/ShaderResources.h>

#include <blue_sky/graphics/Model.h>
#include <blue_sky/graphics/Line.h>

#include <core/math/Quaternion.h>
#include <core/sound/Sound.h>
#include <core/animation/AnimationPlayer.h>

#include <game/Shader.h>

#include <common/math.h>

#include <sstream>

namespace blue_sky
{

ActiveObject::ActiveObject()
	: model_( 0 )
	, object_constant_buffer_( new ObjectConstantBuffer( GameMain::get_instance()->get_direct_3d() ) )
	, animation_player_( 0 )
	, is_dead_( false )
	, flicker_scale_( 1.f )

	, direction_degree_( 0 )

	, start_location_( Vector::Zero )
	, start_rotation_( Vector::Zero )
	, start_direction_degree_( 0 )

	, front_( Vector::Forward )
	, right_( Vector::Right )

	, is_mesh_visible_( true )
	, is_line_visible_( true )
{
	
}

ActiveObject::ActiveObject( const ActiveObject& o )
	: GameObject( o )
	, model_( o.model_ )
	, object_constant_buffer_( new ObjectConstantBuffer( GameMain::get_instance()->get_direct_3d() ) )
	, animation_player_( 0 )
	, is_dead_( o.is_dead_ )
	, flicker_scale_( o.flicker_scale_ )

	, direction_degree_( o.direction_degree_ )

	, start_location_( o.start_location_ )
	, start_rotation_( o.start_rotation_ )
	, start_direction_degree_( o.start_direction_degree_ )

	, front_( o.front_ )
	, right_( o.right_ )

	, is_mesh_visible_( o.is_mesh_visible_ )
	, is_line_visible_( o.is_line_visible_ )
{
	setup_animation_player();
}

ActiveObject::~ActiveObject()
{
	delete object_constant_buffer_;
	delete animation_player_;
}

/**
 * アニメーション再生をセットアップする
 *
 */
void ActiveObject::setup_animation_player()
{
	if ( animation_player_ )
	{
		return;
	}

	if ( get_model() && get_model()->get_skinning_animation_set() )
	{
		animation_player_ = new AnimationPlayer( get_model()->get_skinning_animation_set() );
	}
}

void ActiveObject::restart()
{
	is_dead_ = false;
	flicker_scale_ = 1.f;
	
	if ( get_rigid_body() )
	{
		get_transform().set_position( start_location_ );
 		get_transform().set_rotation( Quaternion( math::degree_to_radian( start_rotation_.x() ), math::degree_to_radian( start_rotation_.y() ), math::degree_to_radian( start_rotation_.z() ) ) );

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

	Quaternion q;
	q.set_euler_zyx( math::degree_to_radian( start_rotation_.z() ), math::degree_to_radian( start_rotation_.y() ), math::degree_to_radian( start_rotation_.x() ) );
	get_transform().set_rotation( q );
}

void ActiveObject::set_start_direction_degree( float d )
{
	start_direction_degree_ = d;
}

void ActiveObject::set_direction_degree( float d )
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
		Quaternion q;
		q.set_euler_zyx( 0.f, math::degree_to_radian( direction_degree_ ), 0.f );
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

void ActiveObject::kill()
{
	is_dead_ = true;

	is_mesh_visible_ = false;
	is_line_visible_ = false;
}

/**
 * 描画用の定数バッファを更新する
 *
 */
void ActiveObject::update_render_data() const
{
	if ( ! is_visible() )
	{
		return;
	}

	const Transform& t = get_transform();

	ObjectShaderResourceData shader_data;

	shader_data.world.set_rotation_quaternion( t.get_rotation() );
	shader_data.world *= Matrix().set_translation( t.get_position().x(), t.get_position().y(), t.get_position().z() );
	shader_data.world = shader_data.world.transpose();

	if ( get_model()->get_line() )
	{
		shader_data.color = get_model()->get_line()->get_color();
	}

	get_object_constant_buffer()->update( & shader_data );

	if ( get_animation_player() )
	{
		get_animation_player()->update_render_data();
	}
}

/**
 * 描画用の定数バッファをシェーダーに設定する
 *
 * @todo 必要なシェーダーにだけ設定できるようにする？
 */
void ActiveObject::bind_render_data() const
{
	get_object_constant_buffer()->bind_to_vs();
	get_object_constant_buffer()->bind_to_ds();
	// get_object_constant_buffer()->bind_to_gs();
	get_object_constant_buffer()->bind_to_ps(); // 必要？
}

/**
 * メッシュを描画する
 *
 */
void ActiveObject::render_mesh() const
{
	if ( ! is_mesh_visible() )
	{
		return;
	}

	if ( ! get_model()->get_mesh() )
	{
		return;
	}

	/// @todo 削除する
	bind_render_data();
	
	if ( get_animation_player() )
	{
		get_animation_player()->bind_render_data();
	}

	get_model()->render();
}

/**
 * 指定したシェーダーでメッシュを描画する
 *
 */
void ActiveObject::render_mesh( const Shader* shader ) const
{
	if ( ! is_mesh_visible() )
	{
		return;
	}

	if ( ! get_model()->get_mesh() )
	{
		return;
	}

	if ( get_animation_player() )
	{
		get_animation_player()->bind_render_data();
	}

	shader->render_model( get_model() );
}


/**
 * 線を描画する
 *
 */
void ActiveObject::render_line() const
{
	if ( ! is_line_visible() )
	{
		return;
	}

	if ( ! get_model()->get_line() )
	{
		return;
	}

	bind_render_data();

	get_model()->get_line()->render();
}

void ActiveObject::play_animation( const char* name, bool force, bool loop )
{
	if ( ! get_animation_player() )
	{
		return;
	}

	get_animation_player()->play( name, force, loop );
}

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
