#include "ActiveObject.h"
#include "Stage.h"
#include "GridCell.h"

#include "GameMain.h"

#include "Sound.h"
#include "SoundManager.h"
#include "Input.h"

#include "matrix4x4.h"

#include <game/AABB.h>

#include <common/exception.h>
#include <common/math.h>

#include <list>

namespace blue_sky
{

ActiveObject::ActiveObject()
	 : stage_( 0 )
	 , direction_degree_( 0.f )
	 , start_direction_degree_( 0.f )
	 , floor_cell_( 0 )
	 , is_dead_( false )
{
	set_direction_degree( 0.f );
}

float ActiveObject::get_collision_width() const
{
	return 0.4f;
}

float ActiveObject::get_collision_height() const
{
	return 1.5f;
}

float ActiveObject::get_collision_depth() const
{
	return 0.4f;
}

void ActiveObject::setup_local_aabb_list()
{
	vector3 min( -get_collision_width() * 0.5f, 0.f, -get_collision_depth() * 0.5f );
	vector3 max( get_collision_width() * 0.5f, get_collision_height(), get_collision_depth() * 0.5f );

	local_aabb_list_.clear();
	local_aabb_list_.push_back( AABB( min, max ) );
}

void ActiveObject::set_direction_degree( float d )
{
	direction_degree_ = d;

	while ( direction_degree_ < 0.f ) direction_degree_ += 360.f;
	while ( direction_degree_ > 360.f ) direction_degree_ -= 360.f;

	if ( direction_degree_ < 45.f ) direction_ = FRONT;
	else if ( direction_degree_ < 45.f + 90.f * 1.f ) direction_ = RIGHT;
	else if ( direction_degree_ < 45.f + 90.f * 2.f ) direction_ = BACK;
	else if ( direction_degree_ < 45.f + 90.f * 3.f ) direction_ = LEFT;
	else direction_ = FRONT;

	matrix4x4 m;
	m.rotate_y( direction_degree_ );

	front_ = vector3( 0.f, 0.f, 1.f ) * m;
	right_ = vector3( 1.f, 0.f, 0.f ) * m;
}

const GridCell& ActiveObject::get_floor_cell_center() const
{
	return stage_->cell( static_cast< int >( position_.x() ), static_cast< int >( position_.z() ) );
}

const GridCell& ActiveObject::get_floor_cell_left_front() const
{
	return stage_->cell( static_cast< int >( position().x() - std::min( get_collision_width() * 0.5f, 0.95f ) ), static_cast< int >( position().z() + std::min( get_collision_depth() * 0.5f, 0.95f ) ) );
}

const GridCell& ActiveObject::get_floor_cell_right_front() const
{
	return stage_->cell( static_cast< int >( position().x() + std::min( get_collision_width() * 0.5f, 0.95f ) ), static_cast< int >( position().z() + std::min( get_collision_depth() * 0.5f, 0.95f ) ) );
}

const GridCell& ActiveObject::get_floor_cell_left_back() const
{
	return stage_->cell( static_cast< int >( position().x() - std::min( get_collision_width() * 0.5f, 0.95f ) ), static_cast< int >( position().z() - std::min( get_collision_depth() * 0.5f, 0.95f ) ) );
}

const GridCell& ActiveObject::get_floor_cell_right_back() const
{
	return stage_->cell( static_cast< int >( position().x() + std::min( get_collision_width() * 0.5f, 0.95f ) ), static_cast< int >( position().z() - std::min( get_collision_depth() * 0.5f, 0.95f ) ) );
}

void ActiveObject::set_stage( const Stage* stage )
{
	stage_ = stage;
}

void ActiveObject::limit_velocity()
{
	velocity().x() = math::clamp( velocity().x(), -get_max_speed(), get_max_speed() );
	velocity().y() = math::clamp( velocity().y(), -get_max_speed(), get_max_speed() );
	velocity().z() = math::clamp( velocity().z(), -get_max_speed(), get_max_speed() );
}

void ActiveObject::update_position()
{
	const vector3 last_position = position();

	position().x() += velocity().x();
	position().x() = math::clamp( position().x(), 0.f, static_cast< float >( stage()->width() ) );

	const GridCell& floor_cell_x = update_floor_cell();

	if ( position().y() < floor_cell_x.height() )
	{
		on_collision_x( floor_cell_x );
		position().x() = last_position.x();
	}

	position().z() += velocity().z();
	position().z() = math::clamp( position().z(), 0.f, static_cast< float >( stage()->depth() ) );

	const GridCell& floor_cell_z = update_floor_cell();

	if ( position().y() < floor_cell_z.height() )
	{
		on_collision_z( floor_cell_z );
		position().z() = last_position.z();
	}

	position().y() += velocity().y();

	if ( position().y() >= 300.f )
	{
		position().y() = 300.f;
		velocity().y() = 0.f;
	}

	const GridCell& floor_cell_y = update_floor_cell();
	
	if ( position().y() < floor_cell_y.height() )
	{
		position().y() = floor_cell_y.height();

		on_collision_y( floor_cell_y );
	}

	position().y() = std::max( 0.f, position().y() );

	/// @todo rotate
	update_global_aabb_list();
}

void ActiveObject::limit_position()
{
	position().x() = math::clamp( position().x(), 0.f, static_cast< float >( stage()->width() ) );
	position().y() = math::clamp( position().y(), 0.f, 300.f );
	position().z() = math::clamp( position().z(), 0.f, static_cast< float >( stage()->depth() ) );
}

void ActiveObject::update_global_aabb_list()
{
	global_aabb_list_.clear();

	for ( AABBList::iterator i = local_aabb_list_.begin(); i != local_aabb_list_.end(); ++i )
	{
		global_aabb_list_.push_back( *i + position() );
	}
}

bool ActiveObject::collision_detection( const ActiveObject* active_object ) const
{
	for ( AABBList::const_iterator i = global_aabb_list().begin(); i != global_aabb_list().end(); ++i )
	{
		for ( AABBList::const_iterator j = active_object->global_aabb_list().begin(); j != active_object->global_aabb_list().end(); ++j )
		{
			if ( i->collision_detection( *j ) )
			{
				return true;
			}
		}
	}
	
	return false;
}

void ActiveObject::kill()
{
	is_dead_ = true;
}

void ActiveObject::restart()
{
	is_dead_ = false;

	position() = start_position();
	set_direction_degree( start_direction_degree_ );

	setup_local_aabb_list();
	update_global_aabb_list();
}

/**
 * オブジェクトの接触している一番高いグリッドセルを返す
 */
const GridCell& ActiveObject::update_floor_cell()
{
	std::list< const GridCell* > grid_cell_list;

	grid_cell_list.push_back( & get_floor_cell_left_front() );
	grid_cell_list.push_back( & get_floor_cell_right_front() );
	grid_cell_list.push_back( & get_floor_cell_left_back() );
	grid_cell_list.push_back( & get_floor_cell_right_back() );

	grid_cell_list.sort( GridCell::height_less() );

	floor_cell_ = grid_cell_list.back();

	return * floor_cell_;
}

void ActiveObject::play_sound( const char* name, bool loop, bool force ) const
{
	Sound* sound = GameMain::getInstance()->get_sound_manager()->get_sound( name );
	
	if ( sound )
	{
		if ( force || ! sound->is_playing() )
		{
			sound->set_3d_position( position().x(), position().y(), position().z() );
			sound->set_3d_velocity( velocity().x() * 60.f, velocity().z() * 60.f, velocity().z() * 60.f );
			sound->play( loop );
		}
	}
}

void ActiveObject::stop_sound( const char* name ) const
{
	Sound* sound = GameMain::getInstance()->get_sound_manager()->get_sound( name );
	
	if ( sound )
	{
		sound->stop();
	}
}

} // namespace blue_sky
