#include "Balloon.h"
#include "Player.h"
#include "DrawingModel.h"
#include "DrawingLine.h"
#include <common/random.h>

#include "memory.h"

namespace blue_sky
{

Balloon::Balloon()
	: player_( 0 )
	, flicker_( 0 )
{

}

void Balloon::set_drawing_model( const DrawingModel* m )
{
	ActiveObject::set_drawing_model( m );

	if ( m->get_line() )
	{
		m->get_line()->set_cast_shadow( true );
	}
}

/**
 * 更新
 *
 */
void Balloon::update()
{
	flicker_ += 0.02f;

	if ( player_ )
	{
		if ( player_->get_balloon() == this )
		{
			set_location( player_->get_location() + player_->get_front() * 0.25f - player_->get_right() * 0.25f + Vector3( 0, 1.75f + std::sin( flicker_ ) * 0.1f, 0 ) );
			set_direction_degree( 0 );

			if ( player_->get_action_mode() != Player::ACTION_MODE_BALLOON )
			{
				kill();
			}
		}
		else
		{
			kill();
		}
	}
	else
	{
		set_location( get_location().x(), get_start_location().y() + std::sin( flicker_ ) * 0.5f, get_location().z() );
		set_direction_degree( get_direction_degree() + 1.f );
	}
}

void Balloon::restart()
{
	ActiveObject::restart();

	flicker_ = common::random( 0.f, 10.f );
	player_ = 0;
}

void Balloon::kill()
{
	ActiveObject::kill();

	// 影響のの無い場所に退避させる
	set_location( 0.f, -100.f, 0.f );
}

/**
 * オブジェクトが表示されるかを返す
 *
 * @return bool オブジェクトが表示される場合は true を、表示されない場合は false を返す
 */
bool Balloon::is_visible() const
{
	return ActiveObject::is_visible() && ( ! player_ || ! player_->is_action_pre_finish() || is_visible_in_blink( 6.f / 1.f ) );
}

void Balloon::on_collide_with( Stone* )
{
	kill();
	play_sound( "balloon-burst" );
}

} // namespace blue_sky
