#include "Canvas.h"

#define _USE_MATH_DEFINES

#include <math.h>

namespace art
{

Canvas::Vertex::Vertex()
	: angle_( 0.f )
{
	//
}

void Canvas::Vertex::update()
{
	static float a = 0.f;
	a += 0.01f;

	art::Vertex d = target_vertex() - vertex();
	art::Vertex r( rand() % 100 / 100.f * 2.f - 1.f, rand() % 100 / 100.f * 2.f - 1.f, rand() % 100 / 100.f * 2.f - 1.f );

	direction() = ( direction() * 14 + d * 1 + r * 1 ) / 16;

	// direction() *= ( sin( a ) + 1.f ) * 0.1f;

	/*
	float rx = target_vertex().x() - vertex().x();
	float ry = target_vertex().y() - vertex().y();
	
	float len = sqrt( rx * rx + ry * ry );
	float a = atan2( ry, rx );
	
	while ( a < 0 )
	{
		a += 2 * static_cast<float>( M_PI );
	}
	while ( a >= 2 * static_cast<float>( M_PI ) )
	{
		a -= 2 * static_cast<float>( M_PI ) ;
	}
	
	while ( angle_ < 0 )
	{
		angle_ += 2 * static_cast<float>( M_PI );
	}
	while ( angle_ >= 2 * static_cast<float>( M_PI ) )
	{
		angle_ -= 2 * static_cast<float>( M_PI );
	}

	float aa = a - angle_;
	float direction_fix = 0.1f;

	while ( aa < 0 )
	{
		aa += ( 2 * static_cast<float>( M_PI ) );
	}

	if ( aa == 0 )
	{
		
	}
	else if ( aa <= static_cast<float>( M_PI ) )
	{
		angle_ += direction_fix;
	}
	else if ( aa > static_cast<float>( M_PI ) )
	{
		angle_ -= direction_fix;
	}
	else
	{
		
	}

	float diff = abs( static_cast<float>( M_PI ) - aa );

	direction_.x() = cos( angle_ ) * min( diff * 0.1f, len );
	direction_.y() = sin( angle_ ) * min( diff * 0.1f, len );
	*/

	vertex_ += direction_;
}

}; // namespace art