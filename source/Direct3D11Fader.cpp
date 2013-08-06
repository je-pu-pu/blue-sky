#include "Direct3D11Fader.h"
#include "DirectX.h"

#include <common/math.h>

#include "memory.h"

Direct3D11Fader::Direct3D11Fader( Direct3D* direct_3d )
	: Direct3D11Rectangle( direct_3d )
	, fade_color_( 1.f, 1.f, 1.f, 1.f )
	, current_color_( 1.f, 1.f, 1.f, 1.f )
{	

}

bool Direct3D11Fader::fade_in( float speed )
{
	fade_ = math::chase( fade_, 0.f, speed );

	update_color();

	return fade_ == 0.f;
}

bool Direct3D11Fader::fade_out( float speed )
{
	fade_ = math::chase( fade_, 1.f, speed );

	update_color();

	return fade_ == 1.f;
}

void Direct3D11Fader::full_in()
{
	fade_ = 0.f;

	update_color();
}

void Direct3D11Fader::full_out()
{
	fade_ = 1.f;

	update_color();
}

void Direct3D11Fader::update_color()
{
	current_color_.a() = fade_color_.a() * fade_;
}