#include "TranslationObject.h"

#include <common/math.h>

#include <btBulletDynamicsCommon.h>

namespace blue_sky
{

TranslationObject::TranslationObject( float_t w, float_t h, float_t d, float_t tw, float_t th, float_t td, float_t s )
	: StaticObject( w, h, d )
	, tw_( tw )
	, th_( th )
	, td_( td )
	, a_( 0.f )
	, speed_( s )
{
	
}

TranslationObject::~TranslationObject()
{
	
}

void TranslationObject::update()
{
	get_rigid_body()->setActivationState( true );

	a_ += get_frame_elapsed_time() * speed_;

	set_location( get_start_location() + Vector( std::cos( a_ ) * tw_, std::sin( a_ ) * th_, std::cos( a_ ) * td_ ) );
}

} // namespace blue_sky
