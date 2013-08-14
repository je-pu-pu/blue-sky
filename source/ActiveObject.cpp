#include "ActiveObject.h"
#include "ActiveObjectPhysics.h"
#include "SoundManager.h"
#include "Sound.h"
#include "DrawingModel.h"
#include "AnimationPlayer.h"
#include "GameMain.h"

#include "DrawingModel.h"
#include "DrawingMesh.h"
#include "DrawingLine.h"

#include <game/Material.h>
#include <game/Texture.h>

#include <common/math.h>

#include <sstream>

#include "memory.h"

namespace blue_sky
{

ActiveObject::ActiveObject()
	: drawing_model_( 0 )
	, object_constant_buffer_( new ObjectConstantBuffer( GameMain::get_instance()->get_direct_3d() ) )
	, animation_player_( 0 )
	, is_dead_( false )
	, flicker_scale_( 1.f )

	, direction_degree_( 0 )

	, start_location_( 0, 0, 0 )
	, start_rotation_( 0, 0, 0 )
	, start_direction_degree_( 0 )

	, front_( 0, 0, 1 )
	, right_( 1, 0, 0 )

	, is_mesh_visible_( true )
	, is_line_visible_( true )
{
	
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

	if ( get_drawing_model()->has_animation() )
	{
		animation_player_ = get_drawing_model()->create_animation_player();
	}
}

void ActiveObject::restart()
{
	is_dead_ = false;
	flicker_scale_ = 1.f;
	
	if ( get_rigid_body() )
	{
		get_transform().setOrigin( start_location_ );
 		get_transform().setRotation( Quaternion( math::degree_to_radian( start_rotation_.x() ), math::degree_to_radian( start_rotation_.y() ), math::degree_to_radian( start_rotation_.z() ) ) );

		get_rigid_body()->activate( true );
		get_rigid_body()->getMotionState()->setWorldTransform( get_transform() );
		
		get_rigid_body()->setWorldTransform( get_transform() );
		get_rigid_body()->setInterpolationWorldTransform( get_transform() );
		
		get_rigid_body()->setLinearVelocity( Vector3( 0.f, 0.f, 0.f ) );
		get_rigid_body()->setInterpolationLinearVelocity( Vector3( 0.f, 0.f, 0.f ) );
		
		get_rigid_body()->setAngularVelocity( Vector3( 0.f, 0.f, 0.f ) );
		get_rigid_body()->setInterpolationAngularVelocity( Vector3( 0.f, 0.f, 0.f ) );
		
		get_rigid_body()->setGravity( get_default_gravity() );

		get_rigid_body()->clearForces();

		set_direction_degree( start_direction_degree_ );
	}

	is_mesh_visible_ = true;
	is_line_visible_ = true;
}

void ActiveObject::limit_velocity()
{
	Vector3 v = get_rigid_body()->getLinearVelocity();

	v.setX( math::clamp( v.x(), -get_max_speed(), get_max_speed() ) );
	v.setZ( math::clamp( v.z(), -get_max_speed(), get_max_speed() ) );

	// debug
	// v.setY( math::clamp( v.y(), -0.1f, 0.1f ) );

	set_velocity( v );
}

void ActiveObject::set_start_location( float_t x, float_t y, float_t z )
{
	start_location_.setValue( x, y, z );
	get_transform().getOrigin() = start_location_;
}

void ActiveObject::set_start_rotation( float_t x, float_t y, float_t z )
{
	start_rotation_.setValue( x, y, z );

	Quaternion q;
	q.setEulerZYX( math::degree_to_radian( start_rotation_.z() ), math::degree_to_radian( start_rotation_.y() ), math::degree_to_radian( start_rotation_.x() ) );
	get_transform().setRotation( q );
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
		m.setEulerZYX( 0, math::degree_to_radian( -direction_degree_ ), 0 );

		front_ = Vector3( 0.f, 0.f, 1.f ) * m;
		right_ = Vector3( 1.f, 0.f, 0.f ) * m;

		front_.normalize();
		right_.normalize();
	}

	{
		Quaternion q;
		q.setEulerZYX( 0.f, math::degree_to_radian( direction_degree_ ), 0.f );
		get_transform().setRotation( q );

		commit_transform();
	}
}

/**
 * 方向を指定した場所の方向に近づける
 *
 * @param location 目的の場所
 * @param speed 速度
 */
void ActiveObject::chase_direction_to( const Vector3& location, float_t speed )
{
	Vector3 relative_position = location - get_location();
	relative_position.setY( 0 );

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

	/*
	if ( get_rigid_body() )
	{
		set_location( 0.f, -100.f, 0.f );
	}
	*/
}

void ActiveObject::render_mesh() const
{
	if ( ! is_mesh_visible() )
	{
		return;
	}

	get_object_constant_buffer()->bind_to_vs();
	get_object_constant_buffer()->bind_to_ps();
	
	if ( get_animation_player() )
	{
		get_animation_player()->bind_render_data();
	}

	get_drawing_model()->get_mesh()->bind_to_ia();

	for ( uint_t n = 0; n < get_drawing_model()->get_mesh()->get_material_count(); ++n )
	{
		render_material_at( n );
	}
}

void ActiveObject::render_material_at( uint_t material_index ) const
{
	game::Material* material = get_drawing_model()->get_mesh()->get_material_at( material_index );

	if ( material->get_texture() )
	{
		material->get_texture()->bind_to_ps( 0 );
	}

	material->bind_to_ia();
	material->render();
}

void ActiveObject::render_line() const
{
	if ( ! is_line_visible() )
	{
		return;
	}

	if ( ! get_drawing_model()->get_line() )
	{
		return;
	}

	get_object_constant_buffer()->bind_to_vs();
	get_object_constant_buffer()->bind_to_ps();

	get_drawing_model()->get_line()->render(); // 200 + static_cast< int >( XMVectorGetZ( eye ) * 10.f ) );
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
