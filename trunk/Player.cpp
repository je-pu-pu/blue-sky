#include "Player.h"
#include "Stage.h"
#include "GridCell.h"

#include "GameMain.h"

#include "Sound.h"
#include "SoundManager.h"
#include "Input.h"

#include "Direct3D9.h"

#include "matrix4x4.h"

#include <game/AABB.h>

#include <common/exception.h>
#include <common/math.h>

#include <list>

namespace blue_sky
{

Player::Player()
	: input_( 0 )
	, step_count_( 0 )
	, step_speed_( 0.05f )
	, gravity_( 0.01f )
	, super_jump_velocity_( 0.f )
	, eye_height_( 1.5f )
	, is_turn_avaiable_( true )
	, is_jumping_( false )
	, is_clambering_( false )
	, is_falling_( false )
	, action_mode_( ACTION_MODE_NONE )
	, is_action_pre_finish_( false )
	, has_medal_( false )
	, selected_item_type_( ITEM_TYPE_NONE )
	, look_floor_request_( false )
	, last_floor_height_( 300.f )
{
	setup_local_aabb_list();

	rebirth();
}

void Player::step( float s )
{
	if ( is_rocketing() || is_falling() || is_last_floor_height_null() )
	{
		return;
	}

	step_count_++;

	update_step_speed();

	velocity() += front() * s * get_step_speed();
}

void Player::side_step( float s )
{
	if ( is_rocketing() || is_falling() || is_last_floor_height_null() )
	{
		return;
	}

	velocity() += right() * s * get_side_step_speed();
}

void Player::stop()
{
	if ( is_jumping() )
	{
		return;
	}

	step_count_ = 0;

	update_step_speed();
}

void Player::update_step_speed()
{
	if ( is_jumping() || action_mode_ == ACTION_MODE_BALLOON )
	{
		step_speed_ = 0.012f; // math::chase( step_speed_, 0.1f, 0.001f );

		stop_sound( "walk" );
		stop_sound( "run" );

		return;
	}
	
	if ( step_count_ <= 0 )
	{
		// stop
		step_speed_ = 0.f;
	}
	else if ( step_count_ <= 60 )
	{
		// slow walk
		step_speed_ = math::chase( step_speed_, 0.001f, 0.001f );
	}
	else if ( step_count_ <= 180 )
	{
		// slow walk
		step_speed_ = math::chase( step_speed_, 0.005f, 0.001f );
	}
	else
	{
		// run
		step_speed_ = math::chase( step_speed_, 0.01f, 0.001f );
	}

	if ( is_jumping() || is_rocketing() || is_clambering() )
	{
		stop_sound( "walk" );
		stop_sound( "run" );
	}
	else if ( step_count_ >= 20 && step_count_ < 240 )
	{
		play_sound( "walk", true, false );
	}
	else if ( step_count_ >= 240 )
	{
		stop_sound( "walk" );
		play_sound( "run", true, false );
	}
	else if ( step_count_ == 0 )
	{
		stop_sound( "walk" );
		stop_sound( "run" );
	}
}

/**
 *
 */
void Player::add_direction_degree( float d )
{
	if ( is_rocketing() )
	{
		return;
	}

	set_direction_degree( get_direction_degree() + d );
}

/**
 * �X�V
 *
 */
void Player::update()
{
	limit_velocity();
	update_position();

	// �d��
	if ( action_mode_ == ACTION_MODE_BALLOON )
	{
		// ���D
		velocity().y() = math::chase( velocity().y(), 0.08f, 0.02f );

		if ( position().y() - action_base_position_.y() >= get_balloon_action_length() )
		{
			play_sound( "balloon-burst" );
			set_action_mode( ACTION_MODE_NONE );
			is_jumping_ = true;
			is_action_pre_finish_ = false;
		}
		else if ( position().y() - action_base_position_.y() >= get_balloon_action_length() * 0.5f )
		{
			is_action_pre_finish_ = true;
		}
	}
	else if ( action_mode_ == ACTION_MODE_ROCKET )
	{
		// ���P�b�g
		if ( ( position() - action_base_position_ ).length() >= get_rocket_action_length() )
		{
			play_sound( "rocket-burst" );
			set_action_mode( ACTION_MODE_NONE );
			is_jumping_ = true;
			is_action_pre_finish_ = false;
		}
		else if ( ( position() - action_base_position_ ).length() >= get_rocket_action_length() * 0.8f )
		{
			is_action_pre_finish_ = true;
		}
	}
	else if ( action_mode_ == ACTION_MODE_UMBRELLA && velocity().y() < -0.1f )
	{
		// �ӂ��藎��
		velocity().y() = math::chase( velocity().y(), -0.1f, 0.02f );

		if ( position().y() < 40.f )
		{
			set_action_mode( ACTION_MODE_NONE );
			is_jumping_ = true;
			is_action_pre_finish_ = false;
		}
		else if ( position().y() < 48.f )
		{
			is_action_pre_finish_ = true;
		}
	}
	else
	{
		// �ʏ�
		if ( is_jumping() && ! input_->press( Input::A ) && velocity().y() > 0.f )
		{
			velocity().y() *= 0.9f;
		}

		velocity().y() -= get_gravity();
	}

	// ����
	if ( ! is_rocketing() )
	{
		if ( is_jumping() )
		{
			velocity().x() *= 0.91f;
			velocity().z() *= 0.91f;
		}
		else
		{
			velocity().x() *= 0.9f;
			velocity().z() *= 0.9f;
		}

		if ( velocity().y() < -get_max_speed() * 0.02f && action_mode_ != ACTION_MODE_BALLOON )
		{
			is_jumping_ = true;
		}
	}

	if ( is_dead() )
	{
		eye_height_ -= 0.02f;
		eye_height_ = std::max( 0.2f, eye_height_ );
	}
}

void Player::on_collision_x( const GridCell& floor_cell_x )
{
	if (
		( floor_cell_x.height() - position().y() <= 2.f ) &&
		(
			( velocity().x() < 0.f && direction() == LEFT || velocity().x() > 0.f && direction() == RIGHT ) ||
			( floor_cell_x.height() - position().y() <= 1.f && ( velocity().x() < 0.f && direction() != RIGHT || velocity().x() > 0.f && direction() != LEFT ) )
		) )
	{
		velocity().y() = get_clambering_speed();

		is_jumping_ = false;
		// step_count_ = 0;
		// step_speed_ = 0.f;

		if ( ! is_clambering() && floor_cell_x.height() - position().y() > 1.f )
		{
			play_sound( "clamber" );
		}

		is_clambering_ = true;
	}
	else
	{
		play_sound( "collision-wall", false, false );

		if ( is_rocketing() )
		{
			stop_rocket();
			velocity().x() *= -0.8f;
		}
		
		if ( is_if_fall_to_umbrella_lost( floor_cell_x.height() ) )
		{
			lose_umbrella();
		}
	}

	if ( ! is_rocketing() )
	{
		velocity().x() *= 0.1f;
	}

	push_look_floor_request();
}

void Player::on_collision_y( const GridCell& floor_cell_y )
{
	// if ( is_jumping() && is_falling() && floor_cell_y.bound() > 0 )
	if ( is_jumping() && floor_cell_y.bound() > 0 )
	{
		// �X�[�p�[�W�����v
		super_jump_velocity_ += 0.5f;

		velocity().y() = super_jump_velocity_;
	
		is_jumping_ = true;

		stop_sound( "super-jump" );
		play_sound( "super-jump" );

		float speed = 0.f;

		if ( is_falling_ )
		{
			speed = 0.1f; // velocity_on_fall_.length_xz();
		}
		else
		{
			speed = velocity().length_xz();
		}

		velocity().x() = 0.f;
		velocity().z() = 0.f;
		velocity() += front() * speed;
	}
	else
	{
		// ���n���s
		if ( is_falling_to_dead() )
		{
			kill();
			play_sound( "land" );
		}

		// �ʏ풅�n
		if ( velocity().y() < -0.01f )
		{
			stop_sound( "jump" );
			play_sound( "land" );

			is_jumping_ = false;
		}

		velocity().y() = 0.f;
		super_jump_velocity_ = 0.f;

		/*
		if ( is_jumping_ )
		{
			stop_sound( "jump" );
			play_sound( "land" );

			is_jumping_ = false;
		}
		*/

		if ( is_if_fall_to_umbrella_lost( floor_cell_y.height() ) )
		{
			lose_umbrella();
		}
	}

	is_turn_avaiable_ = true;
	is_falling_ = false;
	is_clambering_ = false;

	last_floor_height_ = floor_cell_y.height();
}

void Player::on_collision_z( const GridCell& floor_cell_z )
{
	if (
		( floor_cell_z.height() - position().y() <= 2.f ) && 
		(
			( velocity().z() < 0.f && direction() == BACK || velocity().z() > 0.f && direction() == FRONT ) ||
			( floor_cell_z.height() - position().y() <= 1.f && ( velocity().z() < 0.f && direction() != FRONT || velocity().z() > 0.f && direction() != BACK ) )
		) )
	{
		velocity().y() = get_clambering_speed();

		is_jumping_ = false;
		// step_count_ = 0;
		// step_speed_ = 0.f;

		if ( ! is_clambering() && floor_cell_z.height() - position().y() > 1.f )
		{
			play_sound( "clamber" );
		}

		is_clambering_ = true;
	}
	else
	{
		play_sound( "collision-wall", false, false );

		if ( is_rocketing() )
		{
			stop_rocket();
			velocity().z() *= -0.8f;
		}

		if ( is_if_fall_to_umbrella_lost( floor_cell_z.height() ) )
		{
			lose_umbrella();
		}
	}

	if ( ! is_rocketing() )
	{
		velocity().z() *= 0.1f;
	}

	push_look_floor_request();
}

void Player::lose_umbrella()
{
	if ( action_mode_ == ACTION_MODE_UMBRELLA )
	{
		set_action_mode( ACTION_MODE_NONE );
	}

	item_count_[ ITEM_TYPE_UMBRELLA ]--;

	if ( item_count_[ ITEM_TYPE_UMBRELLA ] <= 0 )
	{
		item_count_[ ITEM_TYPE_UMBRELLA ] = 0;

		if ( selected_item_type_ == ITEM_TYPE_UMBRELLA )
		{
			selected_item_type_ = ITEM_TYPE_NONE;
		}
	}
	else
	{
		selected_item_type_ = ITEM_TYPE_UMBRELLA;
	}
}

/**
 * �W�����v�J�n
 * 
 */	
void Player::jump()
{
	if ( is_jumping() )
	{
		// fall();

		return;
	}
	
	velocity().y() = 0.5f;
	
	is_jumping_ = true;

	if ( action_mode_ == ACTION_MODE_UMBRELLA )
	{
		// if ( player_->get
	}
	else
	{
		set_action_mode( ACTION_MODE_NONE );
	}

	play_sound( "jump" );
}

/**
 * �����J�n
 *
 */
void Player::fall()
{
	if ( is_falling_ ) return;

	is_falling_ = true;

	velocity_on_fall_ = velocity();
	velocity() = vector3( 0.f, -get_max_speed(), 0.f );

	play_sound( "fall" );
}

/**
 * �����P���[�h�J�n
 *
 */
void Player::start_umbrella_mode()
{
	if ( action_mode_ == ACTION_MODE_UMBRELLA )
	{
		set_action_mode( ACTION_MODE_NONE );
	}
	else
	{
		set_action_mode( ACTION_MODE_UMBRELLA );
		selected_item_type_ = ITEM_TYPE_NONE;
		is_action_pre_finish_ = false;
	}
}

/**
 *
 */
void Player::switch_scope_mode()
{
	if ( action_mode_ == ACTION_MODE_SCOPE )
	{
		set_action_mode( ACTION_MODE_NONE );
		selected_item_type_ = ITEM_TYPE_NONE;
	}
	else
	{
		action_mode_ = ACTION_MODE_SCOPE;
	}
}

/**
 * ���̂܂ܗ�������Ǝ��S���邩�ǂ������擾����
 *
 */
bool Player::is_falling_to_dead() const
{
	if ( action_mode_ == ACTION_MODE_UMBRELLA ) return false;

	if ( is_last_floor_height_null() ) return false;
	if ( ! floor_cell() ) return false;
	
	return velocity().y() < -0.2f && last_floor_height() - floor_cell()->height() >= 45;
}

/**
 * �w�肳�ꂽ�����ɂ��̂܂ܗ�������Ǝ��S���邩�ǂ������擾����
 */
bool Player::is_if_fall_to_dead( float height ) const
{
	if ( action_mode_ == ACTION_MODE_UMBRELLA ) return false;
	if ( is_last_floor_height_null() ) return false;

	return last_floor_height() - height >= 45;
}

/**
 * �w�肳�ꂽ�����ɂ��̂܂ܗ�������ƎP������邩�ǂ������擾����
 */
bool Player::is_if_fall_to_umbrella_lost( float height ) const
{
	// if ( action_mode_ != ACTION_MODE_UMBRELLA ) return false;
	if ( is_last_floor_height_null() ) return false;

	return last_floor_height() - height >= 45;
}

void Player::kill()
{
	if ( ! is_dead() )
	{
		play_sound( "dead" );
	}

	ActiveObject::kill();
}

void Player::rebirth()
{
	eye_height_ = 1.5f;
	
	action_mode_ = ACTION_MODE_NONE;
	has_medal_ = false;

	for ( int n = 0; n < ITEM_TYPE_MAX; n++ )
	{
		item_count_[ n ] = 0;
	}
	item_count_[ ITEM_TYPE_SCOPE ] = 1;

	selected_item_type_ = ITEM_TYPE_NONE;

	velocity().init();

	last_floor_height_ = 300.f;

	restart();
}

void Player::set_input( const Input* input )
{
	input_ = input;
}

void Player::rocket( const vector3& direction )
{
	if ( get_item_count( ITEM_TYPE_ROCKET ) <= 0 )
	{
		return;
	}

	is_jumping_ = false;
	is_falling_ = false;
	is_action_pre_finish_ = false;

	set_action_mode( ACTION_MODE_ROCKET );
	velocity() = direction * get_max_speed() * 0.5f;
	action_base_position_ = position();

	item_count_[ ITEM_TYPE_ROCKET ]--;

	if ( item_count_[ ITEM_TYPE_ROCKET ] <= 0 )
	{
		selected_item_type_ = ITEM_TYPE_NONE;
	}

	play_sound( "rocket" );
}

void Player::stop_rocket()
{
	set_action_mode( ACTION_MODE_NONE );
	is_action_pre_finish_ = false;
}

void Player::on_get_balloon()
{
	is_jumping_ = false;
	is_falling_ = false;
	is_action_pre_finish_ = false;

	set_action_mode( ACTION_MODE_BALLOON );
	action_base_position_ = position();

	last_floor_height_ = position().y();

	play_sound( "balloon-get" );
}

void Player::on_get_rocket()
{
	item_count_[ ITEM_TYPE_ROCKET ]++;
	selected_item_type_ = ITEM_TYPE_ROCKET;

	play_sound( "rocket-get" );
}

void Player::on_get_umbrella()
{
	item_count_[ ITEM_TYPE_UMBRELLA ]++;

	if ( get_action_mode() != ACTION_MODE_UMBRELLA )
	{
		selected_item_type_ = ITEM_TYPE_UMBRELLA;
	}

	play_sound( "umbrella-get" );
}

void Player::on_get_medal()
{
	has_medal_ = true;

	play_sound( "medal-get" );
}

int Player::get_item_count( ItemType item_type ) const
{
	if ( item_type <= ITEM_TYPE_NONE ) return 0;
	if ( item_type >= ITEM_TYPE_MAX ) return 0;

	return item_count_[ item_type ];
}

void Player::select_prev_item()
{
	for ( int type = static_cast< int >( selected_item_type_ ) - 1; type >= ITEM_TYPE_NONE; type-- )
	{
		if ( type == ITEM_TYPE_NONE || item_count_[ type ] )
		{
			selected_item_type_ = static_cast< ItemType >( type );
			play_sound( "click" );
			break;
		}
	}
}

void Player::select_next_item()
{
	for ( int type = static_cast< int >( selected_item_type_ ) + 1; type < ITEM_TYPE_MAX; type++ )
	{
		if ( item_count_[ type ] )
		{
			selected_item_type_ = static_cast< ItemType >( type );
			play_sound( "click" );
			break;
		}
	}
}

float Player::get_collision_width() const
{
	return 0.4f;
}

float Player::get_collision_height() const
{
	return 1.5f;
}

float Player::get_collision_depth() const
{
	return 0.4f;
}

float Player::get_clambering_speed() const
{
	if ( get_action_mode() == ACTION_MODE_UMBRELLA )
	{
		return 0.02f;
	}

	return 0.05f;
}

void Player::set_action_mode( ActionMode action_mode )
{
	if ( action_mode == action_mode_ )
	{
		return;
	}

	/*
	// �����P�I��
	if ( action_mode_ != ACTION_MODE_UMBRELLA && action_mode_ != ACTION_MODE_NONE )
	{
		if ( action_mode == ACTION_MODE_NONE && item_count_[ ITEM_TYPE_UMBRELLA ] && floor_cell() && is_if_fall_to_dead( floor_cell()->height() ) )
		{
			action_mode = ACTION_MODE_UMBRELLA;
		}
	}
	*/

	action_mode_ = action_mode;
}

} // namespace blue_sky