#include "Canvas.h"

#define _USE_MATH_DEFINES

#include <math.h>

bool g_solid = false;
bool g_line = false;

float g_power = 1.f;
float g_power_min = 1.f;
float g_power_max = 5.f;
float g_power_plus = 0.1f;
float g_power_rest = 0.8f;
float g_power_plus_reset = -1.f;

float g_direction_fix_default = 0.0001f;
float g_direction_fix_acceleration = 0.01f;
float g_direction_random = 0.01f;

int g_circle_count = 0;

namespace art
{

Canvas::Canvas()
{

}

Canvas::~Canvas()
{

}

/**
 * è‘‚«•—‚Ìü‚ğ•`‰æ‚·‚é
 *
 * 
 */
void Canvas::drawLineHumanTouch( const art::Vertex& from, const art::Vertex& to, const Color& cl )
{
	RGBQUAD c = cl;
	Real x1 = from.x();
	Real y1 = from.y();
	Real x2 = to.x();
	Real y2 = to.y();

	// int r = rand() % 255;
	// RGBQUAD black = { 0, r, r, 20 };

	// ‚­‚Á‚«‚è
	if ( g_solid )
	{
		c.rgbReserved = 255;
	}

	// –Ê”’‚İ‚Ì–³‚¢ŠG‚É
	if ( g_line )
	{
		return drawLine( x1, y1, x2, y2, c );
	}

	// í‚Éã‚©‚ç‰º‚Ìü‚ğ•`‚­
	if ( y1 > y2 )
	{
		std::swap( x1, x2 );
		std::swap( y1, y2 );
	}

	int division = 100; // ˆê–{‚Ìü‚ğ•`‰æ‚·‚é‚½‚ß‚ÉÅ‘å‰½ŒÂ‚Ì‰~‚ğ•`‰æ‚·‚é‚©
	Real interval = 10.f; // ‰~‚Æ‰~‚ÌŠÔŠu‚ÌÅ‘å’l ( pixel )

	Real power = 1.2f; // •Mˆ³ ( pixel )
	Real power_min = 1.f; // Å’á•Mˆ³ ( pixel )
	Real power_max = 20.f; // Å‘å•Mˆ³ ( pixel )
	Real power_plus = 0.02f; //
	Real power_reset = 0.8f; // •Mˆ³ƒŠƒZƒbƒg‚Ì”{—¦
	Real power_plus_reset = -1.f;

	Real direction = atan2( y2 - y1, x2 - x1 ); // Œ»İ‚Ì•M‚ÌˆÊ’u‚©‚çI—¹“_‚Ü‚Å‚Ì³Šm‚È•ûŒü ( radian )
	Real direction_fix_default = 0.0001f; // •M‚Ì•ûŒü‚Ì•â³’l‚ÌÅ¬’l ( radian )
	Real direction_fix = direction_fix_default; // •M‚Ì•ûŒü‚Ì•â³’l ( radian )
	Real direction_fix_acceleration = 0.01f; // •M‚Ì•ûŒü‚Ì•â³’l‚Ì‰Á‘¬“x ( radian )
	Real direction_random = 0.01f; // •M‚Ì•ûŒü‚Ìƒ‰ƒ“ƒ_ƒ€«

	int color_plus = -2; // F‚Ì•Ï‰»

	power = g_power;
	power_min = g_power_min;
	power_max = g_power_max;
	power_plus = g_power_plus;
	power_reset = g_power_rest;
	power_plus_reset = g_power_plus_reset;

	direction_fix_default = g_direction_fix_default;
	direction_fix_acceleration = g_direction_fix_acceleration;
	direction_random = g_direction_random;

	direction_fix = direction_fix_default;


	Real a = 0.f; // •Mˆ³‚ÌƒAƒŒ
	Real d = direction + ( ( rand() % 100 / 100.f ) - 0.5f ) * direction_random; // Œ»İ‚Ì•M‚Ì•ûŒü ( radian )

	for ( int n = 0; n < division; n++ )
	{
		x1 += cos( d ) * min( power / 2, interval );
		y1 += sin( d ) * min( power / 2, interval );

		drawCircle( art::Vertex( x1, y1, 0.f ), power, c, true );
		g_circle_count++;

		a += 0.01f; // ( ( rand() % 100 / 100.f ) ) * 0.1f;

		power += ( 1.f + a ) * 0.5f * power_plus;
		// power = a * power_plus;

		if ( power > power_max )
		{
			power = power_max;
			power_plus = -power_plus;
		}
		if ( power < power_min )
		{
			power = power_min;
			power_plus = -power_plus;
		}

		Real lx = x2 - x1;
		Real ly = y2 - y1;
		Real len = sqrt( lx * lx + ly * ly );

		direction = atan2( y2 -y1, x2 - x1 );
		
		while ( direction < 0 )
		{
			direction += ( 2 * static_cast< float >( M_PI ) );
		}
		while ( d < 0 )
		{
			d += ( 2 * static_cast< float >( M_PI ) );
		}
		while ( direction >= 2 * static_cast< float >( M_PI ) )
		{
			direction -= 2 * static_cast< float >( M_PI );
		}
		while ( d >= 2 * static_cast< float >( M_PI ) )
		{
			d -= 2 * static_cast< float >( M_PI );
		}

		Real dd = direction - d;

		while ( dd < 0 )
		{
			dd += ( 2 * static_cast< float >( M_PI ) );
		}

		if ( dd == 0 )
		{
			
		}
		else if ( dd <= static_cast< float >( M_PI ) )
		{
			d += direction_fix;

			if ( d > direction )
			{
				d = direction;
			}
		}
		else if ( dd > static_cast< float >( M_PI ) )
		{
			d -= direction_fix;

			if ( d < direction )
			{
				d = direction;
			}
		}
		else
		{
			break;
		}

		if ( len < power )
		{
			break;
		}

		direction_fix += direction_fix_acceleration;

		// if ( rand() % 100 < 10 )
		if ( false )
		{
			direction_fix = direction_fix_default;
			d += ( ( rand() % 100 / 100.f ) - 0.5f ) * direction_random;
			
			power *= power_reset;
			power_plus *= power_plus_reset;

			c.rgbRed   = min( 255, max( 0, int( c.rgbRed )   + color_plus ) );
			c.rgbGreen = min( 255, max( 0, int( c.rgbGreen ) + color_plus ) );
			c.rgbBlue  = min( 255, max( 0, int( c.rgbBlue )  + color_plus ) );
			
			// c.rgbReserved = rand() % 256;
		}

		// c.rgbReserved = static_cast< int >( a * 100 ) % 256;
		
		/*
		if ( d < direction )
		{
			d += direction_fix;

			if ( d > direction ) d = direction;
		}
		if ( d > direction )
		{
			d -= direction_fix;

			if ( d < direction ) d = direction;
		}
		*/

		direction_fix += 0.001f;

		if ( len < 5.f )
		{
			direction_fix = 100;
		}
	}
}

Canvas::Vertex::Vertex()
	: angle_( 0.f )
	, speed_( 0.f )
{
	//
}

void Canvas::Vertex::update()
{
	// static float a = 0.f;
	// a += 0.01f;

	art::Vertex d = target_vertex() - vertex();
	art::Vertex r( rand() % 100 / 100.f * 2.f - 1.f, rand() % 100 / 100.f * 2.f - 1.f, rand() % 100 / 100.f * 2.f - 1.f );

	// direction() = ( direction() * 14 + d * 1 + r * 1 ) / 16;
	// vertex_ += direction_;

	// direction() *= ( sin( a ) + 1.f ) * 0.1f;
	// vertex_ += direction_;

	/// ’¸“_‚ÌÅ‘åˆÚ“®‘¬“x
	const float max_speed = 20.f;

	/// ù‰ñ‚Ì‘¬“x
	// const float turn_speed = 5.f;

	const float rx = target_vertex().x() - vertex().x();
	const float ry = target_vertex().y() - vertex().y();
	
	const float len = sqrt( rx * rx + ry * ry );
	float a = atan2( ry, rx );
	
	// 0 <= a < 2 * M_PI ‚ÉŠÛ‚ß‚é
	while ( a < 0.f )
	{
		a += 2.f * static_cast<float>( M_PI );
	}
	while ( a >= 2.f * static_cast<float>( M_PI ) )
	{
		a -= 2.f * static_cast<float>( M_PI ) ;
	}

	// 0 <= angle_ < 2 * M_PI ‚ÉŠÛ‚ß‚é
	while ( angle_ < 0.f )
	{
		angle_ += 2.f * static_cast<float>( M_PI );
	}
	while ( angle_ >= 2.f * static_cast<float>( M_PI ) )
	{
		angle_ -= 2.f * static_cast<float>( M_PI );
	}

	float aa = a - angle_;
	const float direction_fix = 3.f;

	// 0 <= angle_ < 2 * M_PI ‚ÉŠÛ‚ß‚é
	while ( aa < 0 )
	{
		aa += ( 2 * static_cast<float>( M_PI ) );
	}

	if ( aa == 0.f )
	{
		// ^³–Ê
	}
	else if ( aa <= static_cast<float>( M_PI ) )
	{
		// ¶‰ñ‚è
		angle_ += direction_fix;

		if ( angle_ > a )
		{
			angle_ = a;
		}
	}
	else if ( aa > static_cast<float>( M_PI ) )
	{
		// ‰E‰ñ‚è
		angle_ -= direction_fix;

		if ( angle_ < a )
		{
			angle_ = a;
		}
	}
	else
	{
		
	}



	// float diff = abs( static_cast<float>( M_PI ) - aa );

	const float speed = min( max_speed, len );
	const float speed_fix = 0.01f;

	if ( speed_ < speed )
	{
		speed_ += speed_fix;

		if ( speed_ > speed )
		{
			speed_ = speed;
		}
	}
	if ( speed_ > speed )
	{
		speed_ -= speed_fix;

		if ( speed_ < 0.f )
		{
			speed_ = 0.f;
		}
	}

	direction_.x() = cos( angle_ ) * speed_; // min( diff * 0.1f, len );
	direction_.y() = sin( angle_ ) * speed_; // min( diff * 0.1f, len );

	vertex_ += direction_;
}

}; // namespace art