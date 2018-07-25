#include "Fader.h"

#include <common/math.h>

namespace blue_sky::graphics
{

Fader::Fader()
	: fade_color_( 1.f, 1.f, 1.f, 1.f )
	, current_color_( 1.f, 1.f, 1.f, 1.f )
	, fade_( 0.f )
{	

}

bool Fader::fade_in( float speed )
{
	fade_ = math::chase( fade_, 0.f, speed );

	update_color();

	return fade_ == 0.f;
}

bool Fader::fade_out( float speed )
{
	fade_ = math::chase( fade_, 1.f, speed );

	update_color();

	return fade_ == 1.f;
}

void Fader::full_in()
{
	fade_ = 0.f;

	update_color();
}

void Fader::full_out()
{
	fade_ = 1.f;

	update_color();
}

void Fader::update_color()
{
	current_color_.a() = fade_color_.a() * fade_;
}

} // namespace blue_sky::graphics
