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
 * �X�V
 *
 */
void Balloon::update()
{
	if ( player_ )
	{
		if ( player_->get_balloon() == this )
		{
			set_velocity( Vector3( 0, 0, 0 ) );
			set_location( player_->get_location() + player_->get_front() * 0.5f - player_->get_right() * 0.25f + Vector3( 0, 1.75f, 0 ) );
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
		update_location_by_flicker( get_start_location(), 0.5f );
		set_direction_degree( get_direction_degree() + 1.f );
	}
}

void Balloon::restart()
{
	ActiveObject::restart();

	player_ = 0;
}

void Balloon::kill()
{
	ActiveObject::kill();

	// �e���̖̂����ꏊ�ɑޔ�������
	set_location( 0.f, -100.f, 0.f );
}

/**
 * �I�u�W�F�N�g�̃��b�V�����\������邩��Ԃ�
 *
 * @return bool �I�u�W�F�N�g�̃��b�V�����\�������ꍇ�� true ���A�\������Ȃ��ꍇ�� false ��Ԃ�
 */
bool Balloon::is_mesh_visible() const
{
	return ActiveObject::is_mesh_visible() && is_visible_with_player();
}

/**
 * �I�u�W�F�N�g�̃��C�����\������邩��Ԃ�
 *
 * @return bool �I�u�W�F�N�g�̃��C�����\�������ꍇ�� true ���A�\������Ȃ��ꍇ�� false ��Ԃ�
 */
bool Balloon::is_line_visible() const
{
	return ActiveObject::is_line_visible() && is_visible_with_player();
}

/**
 * ���D���v���C���[�Ɏ�����Ă��鎞�A�\������邩�ǂ�����Ԃ�
 *
 * ���D���v���C���[�Ɏ�����Ă��Ȃ��ꍇ�A���̊֐��� true ��Ԃ��B
 *
 * @param ���D���v���C���[�Ɏ�����Ă��鎞�A�\�����ꂦ��ꍇ�� true ���A�\������Ȃ��ꍇ�� false ��Ԃ�
 */
bool Balloon::is_visible_with_player() const
{
	return ( ! player_ || ! player_->is_action_pre_finish() || is_visible_in_blink( 6.f / 1.f ) );
}

void Balloon::on_collide_with( Stone* )
{
	kill();
	play_sound( "balloon-burst" );
}

} // namespace blue_sky
