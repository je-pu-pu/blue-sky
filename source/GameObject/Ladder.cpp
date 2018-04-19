#include "Ladder.h"
#include "Player.h"
namespace blue_sky
{

void Ladder::restart()
{
	ActiveObject::restart();

	contact_object_cache_.clear();

	set_mass( 0 );
}

void Ladder::update()
{
	contact_object_cache_.update( get_frame_elapsed_time() );
}

void Ladder::on_collide_with( Player* p )
{
	if ( ! contact_object_cache_.is_cached( p ) ) 
	{
		play_sound( "ladder-contact" );
	}

	contact_object_cache_.cache( p, 0.5f );
}

} // namespace blue_sky
