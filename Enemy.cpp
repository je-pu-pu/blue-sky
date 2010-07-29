#include "Enemy.h"
#include "Player.h"

#include <common/math.h>

namespace blue_sky
{

Enemy::Enemy()
	: player_( 0 )
	, mode_( MODE_FIND )
	, counter_( 0 )
{

}

void Enemy::update()
{
	counter_++;

	if ( player_->is_dead() )
	{
		mode_ = MODE_FIND;
	}

	if ( mode_ == MODE_FIND )
	{
		vector3 relative_position = player_->position() - position();

		if ( relative_position.length() < 20.f )
		{
			mode_ = MODE_CHASE;
		}

		set_direction_degree( static_cast< float >( static_cast< int >( get_direction_degree() ) / 90 * 90 ) );
		velocity() += front() * 0.01f;
	}
	else if ( mode_ == MODE_CHASE )
	{
		if ( counter_ >= 10 )
		{
			vector3 relative_position = player_->position() - position();

			if ( relative_position.length() > 40.f )
			{
				mode_ = MODE_FIND;
			}
			else
			{
				set_direction_degree( math::radian_to_degree( std::atan2( relative_position.x(), relative_position.z() ) ) );

				front() = relative_position;
				front().y() = 0.f;
			
				if ( front().length() > 1.f )
				{
					front().normalize();
				}

				counter_ = 0;
			}
		}

		velocity() += front() * 0.01f;
	}
	else if ( mode_ == MODE_DETOUR )
	{
		velocity() = vector3( 0.f, 0.1f, 0.f );

		if ( counter_ >= 60 )
		{
			mode_ = MODE_CHASE;
			counter_ = 0;
		}
	}

	if ( counter_ % 120 == 0 )
	{
		play_sound( "ok" );
		// play_sound( "vending-machine", true, false );
	}

	velocity().y() -= 0.005f;

	update_position();

	velocity().x() *= 0.8f;
	velocity().z() *= 0.8f;
}

void Enemy::on_collision_x( const GridCell& )
{
	velocity().x() = 0.f;

	if ( mode_ == MODE_FIND )
	{
		set_direction_degree( get_direction_degree() + 90.f );
	}
	else if ( mode_ == MODE_CHASE )
	{
		mode_ = MODE_DETOUR;
	}
}

void Enemy::on_collision_y( const GridCell& )
{
	if ( mode_ == MODE_CHASE )
	{
		front() = player_->position() - position();
		front().y() = 0.f;

		if ( front().length() > 1.f )
		{
			front().normalize();
		}
	}
}

void Enemy::on_collision_z( const GridCell& )
{
	velocity().z() = 0.f;

	if ( mode_ == MODE_FIND )
	{
		set_direction_degree( get_direction_degree() + 90.f );
	}
	else if ( mode_ == MODE_CHASE )
	{
		mode_ = MODE_DETOUR;
	}
}

} // namespace blue_sky
