#include "Canvas.h"

#include <common/Random.h>

#include <math.h>
#include <assert.h>

bool g_solid = false;
bool g_line = false;

float g_power = 1.f;
float g_power_min = 1.f;
float g_power_max = 5.f;
float g_power_plus = 0.1f;
float g_power_rest = 0.8f;
float g_power_plus_reset = -1.f;

float g_direction_fix_default = 0.0001f;
float g_direction_fix_acceleration = 0.00001f;
float g_direction_random = 0.01f;

int g_line_count = 0;
int g_circle_count = 0;

namespace art
{

Canvas::Canvas()
	: brush_( 0 )
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
void Canvas::drawLineHumanTouch( const art::Vertex& from, const art::Vertex& to, const Color& c )
{
	const float w = static_cast< float >( width() );
	const float h = static_cast< float >( height() );

	if ( from.x() < 0.f && to.x() < 0.f ) return;
	if ( from.x() > w   && to.x() > w   ) return;
	if ( from.y() < 0.f && to.y() < 0.f ) return;
	if ( from.y() > h   && to.y() > h   ) return;

	g_line_count++;
	
	const Real edge_pos_random = 1.f;

	Color color = c;

	/*
	Real x1 = from.x() + common::random( -edge_pos_random, edge_pos_random );
	Real y1 = from.y() + common::random( -edge_pos_random, edge_pos_random );
	Real x2 = to.x() + common::random( -edge_pos_random, edge_pos_random );
	Real y2 = to.y() + common::random( -edge_pos_random, edge_pos_random );
	*/

	art::Vertex v1 = from + art::Vertex( common::random( -edge_pos_random, edge_pos_random ), common::random( -edge_pos_random, edge_pos_random ) );
	art::Vertex v2 = to + art::Vertex( common::random( -edge_pos_random, edge_pos_random ), common::random( -edge_pos_random, edge_pos_random ) );

	// int r = rand() % 255;
	// RGBQUAD black = { 0, r, r, 20 };

	// ‚­‚Á‚«‚è
	if ( g_solid )
	{
		color.a() = 255;
	}

	// –Ê”’‚İ‚Ì–³‚¢ŠG‚É
	if ( g_line )
	{
		// return drawLine( x, y1, x2, y2, c );
	}

	int division = 1000; // ˆê–{‚Ìü‚ğ•`‰æ‚·‚é‚½‚ß‚ÉÅ‘å‰½ŒÂ‚Ì‰~‚ğ•`‰æ‚·‚é‚©
	
	const Real min_interval = 1.f;		// ‰~‚Æ‰~‚ÌŠÔŠu‚ÌÅ¬’l ( pixel )
	const Real max_interval = 10.f;		// ‰~‚Æ‰~‚ÌŠÔŠu‚ÌÅ‘å’l ( pixel )

	Real power = 1.2f; // •Mˆ³ ( pixel )
	Real power_min = 1.f; // Å’á•Mˆ³ ( pixel )
	Real power_max = 20.f; // Å‘å•Mˆ³ ( pixel )
	Real power_plus = 0.02f; //
	Real power_reset = 0.8f; // •Mˆ³ƒŠƒZƒbƒg‚Ì”{—¦
	Real power_plus_reset = -1.f;

	Real direction = atan2( v2.y() - v1.y(), v2.x() - v1.x() ); // Œ»İ‚Ì•M‚ÌˆÊ’u‚©‚çI—¹“_‚Ü‚Å‚Ì³Šm‚È•ûŒü ( radian )
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

	/*
	if ( brush_ )
	{
		power = brush_->size();
		power_plus = brush_->size_acceleration();
	}
	*/

	Real a = 0.f; // •Mˆ³‚ÌƒAƒŒ
	const Real random_direction = common::random( -direction_random, direction_random );

	// Real d = direction + ( ( rand() % 100 / 100.f ) - 0.5f ) * direction_random; // Œ»İ‚Ì•M‚Ì•ûŒü ( radian )
	
	Real d = direction + direction_random; // Œ»İ‚Ì•M‚Ì•ûŒü ( radian )

	// direction = d;

	const Real dz = ( v2.z() - v1.z() ) / division;

	for ( int n = 0; n < division; n++ )
	{
		// ‰~‚ğ•`‰æ‚·‚é
		drawCircle( v1, power, color, true );
		g_circle_count++;

		Real interval = min( 1.f, max_interval );
		// const Real interval = 5.f;

		v1 += art::Vertex( cos( d ) * interval, sin( d ) * interval, 0.f );

		power += power_plus;

		if ( power > power_max )
		{
			power = power_max;
			// power_plus = -power_plus;
		}
		if ( power < power_min )
		{
			power = power_min;
			// power_plus = -power_plus;
		}

		Real lx = v2.x() - v1.x();
		Real ly = v2.y() - v1.y();
		Real len = sqrt( lx * lx + ly * ly );

		// Œ»İ‚ÌÀ•W‚©‚ç to ‚Ö‚Ì³Šm‚ÈŠp“x
		direction = atan2( ly, lx );
		
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

		// Œ»İ‚ÌŠp“x‚ğ³Šm‚ÈŠp“x‚Ö‹ß•t‚¯‚é
		if ( dd == 0 )
		{
			// ³–Ê
		}
		else if ( dd <= static_cast< float >( M_PI ) )
		{
			// ¶‰ñ‚è
			d += direction_fix;
		}
		else if ( dd > static_cast< float >( M_PI ) )
		{
			// ‰E‰ñ‚è
			d -= direction_fix;
		}
		else
		{
			break;
		}

		// to ‚Ü‚Å•`‰æ‚µ‚½‚çI—¹‚·‚é
		if ( len < interval )
		{
			// ‰~‚ğ•`‰æ‚·‚é
			drawCircle( to, power, color, true );
			g_circle_count++;

			break;
		}

		direction_fix += direction_fix_acceleration;
		d += random_direction * 0.1f;
	}
}

void Canvas::drawLineHumanTouch( Real x1, Real y1, Real x2, Real y2, const Color& color )
{
	drawLineHumanTouch( art::Vertex( x1, y1 ), art::Vertex( x2, y2 ), color );
}

Canvas::Vertex::Vertex()
	: angle_( 0.f )
	, speed_( 0.f )
{
	//
}

void Canvas::Vertex::update()
{
	vertex() = target_vertex();
	return;
		
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

	/// ‰Á‘¬“x
	const float speed_fix = 0.1f;

	/// ù‰ñ‘¬“x
	const float direction_fix = 0.1f;
	

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

	// 0 <= aa < 2 * M_PI ‚ÉŠÛ‚ß‚é
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

	// Šp“x‚Ì³Šm‚³—¦ 0.f .. 1.f
	float angle_diff = a - angle_;

	// 0.f .. 2 * M_PI
	while ( angle_diff < 0 )
	{
		angle_diff += ( 2 * static_cast<float>( M_PI ) );
	}
	if ( angle_diff > static_cast< float >( M_PI ) )
	{
		angle_diff = ( 2 * static_cast< float >( M_PI ) ) - angle_diff;
	}

	const float angle_correct_rate = 1.f - ( angle_diff / static_cast< float >( M_PI ) );

	assert( angle_correct_rate >= 0.f );
	assert( angle_correct_rate <= 1.f );

	// float diff = abs( static_cast<float>( M_PI ) - aa );

	const float speed = min( max_speed, len ) * ( pow( angle_correct_rate, 8 ) );

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

	// direction() = ( direction() * 15 + d * 1 ) / 16;

	vertex_ += direction_;
}

}; // namespace art