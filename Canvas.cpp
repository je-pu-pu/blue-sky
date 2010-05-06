#include "Canvas.h"

#define _USE_MATH_DEFINES

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
float g_direction_fix_acceleration = 0.01f;
float g_direction_random = 0.01f;

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
 * 手書き風の線を描画する
 *
 * 
 */
void Canvas::drawLineHumanTouch( const art::Vertex& from, const art::Vertex& to, const Color& cl )
{
	const float w = static_cast< float >( width() );
	const float h = static_cast< float >( height() );

	if ( from.x() < 0.f && to.x() < 0.f ) return;
	if ( from.x() > w   && to.x() > w   ) return;
	if ( from.y() < 0.f && to.y() < 0.f ) return;
	if ( from.y() > h   && to.y() > h   ) return;

	RGBQUAD c = cl;
	Real x1 = from.x();
	Real y1 = from.y();
	Real x2 = to.x();
	Real y2 = to.y();

	// int r = rand() % 255;
	// RGBQUAD black = { 0, r, r, 20 };

	// くっきり
	if ( g_solid )
	{
		c.rgbReserved = 255;
	}

	// 面白みの無い絵に
	if ( g_line )
	{
		return drawLine( x1, y1, x2, y2, c );
	}

	// 常に上から下の線を描く
	if ( y1 > y2 )
	{
		std::swap( x1, x2 );
		std::swap( y1, y2 );
	}

	int division = 100; // 一本の線を描画するために最大何個の円を描画するか
	Real max_interval = 10.f; // 円と円の間隔の最大値 ( pixel )

	Real power = 1.2f; // 筆圧 ( pixel )
	Real power_min = 1.f; // 最低筆圧 ( pixel )
	Real power_max = 20.f; // 最大筆圧 ( pixel )
	Real power_plus = 0.02f; //
	Real power_reset = 0.8f; // 筆圧リセット時の倍率
	Real power_plus_reset = -1.f;

	Real direction = atan2( y2 - y1, x2 - x1 ); // 現在の筆の位置から終了点までの正確な方向 ( radian )
	Real direction_fix_default = 0.0001f; // 筆の方向の補正値の最小値 ( radian )
	Real direction_fix = direction_fix_default; // 筆の方向の補正値 ( radian )
	Real direction_fix_acceleration = 0.01f; // 筆の方向の補正値の加速度 ( radian )
	Real direction_random = 0.01f; // 筆の方向のランダム性

	int color_plus = -2; // 色の変化

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


	power = brush_->size();
	power_plus = brush_->size_acceleration();

	Real a = 0.f; // 筆圧のアレ
	Real d = direction + ( ( rand() % 100 / 100.f ) - 0.5f ) * direction_random; // 現在の筆の方向 ( radian )

	for ( int n = 0; n < division; n++ )
	{
		const Real interval = 1.f;

		x1 += cos( d ) * min( interval, max_interval );
		y1 += sin( d ) * min( interval, max_interval );

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

	/// 頂点の最大移動速度
	const float max_speed = 20.f;

	/// 加速度
	const float speed_fix = 0.1f;

	/// 旋回速度
	const float direction_fix = 0.1f;
	

	/// 旋回の速度
	// const float turn_speed = 5.f;

	const float rx = target_vertex().x() - vertex().x();
	const float ry = target_vertex().y() - vertex().y();
	
	const float len = sqrt( rx * rx + ry * ry );
	float a = atan2( ry, rx );
	
	// 0 <= a < 2 * M_PI に丸める
	while ( a < 0.f )
	{
		a += 2.f * static_cast<float>( M_PI );
	}
	while ( a >= 2.f * static_cast<float>( M_PI ) )
	{
		a -= 2.f * static_cast<float>( M_PI ) ;
	}

	// 0 <= angle_ < 2 * M_PI に丸める
	while ( angle_ < 0.f )
	{
		angle_ += 2.f * static_cast<float>( M_PI );
	}
	while ( angle_ >= 2.f * static_cast<float>( M_PI ) )
	{
		angle_ -= 2.f * static_cast<float>( M_PI );
	}

	float aa = a - angle_;

	// 0 <= aa < 2 * M_PI に丸める
	while ( aa < 0 )
	{
		aa += ( 2 * static_cast<float>( M_PI ) );
	}

	if ( aa == 0.f )
	{
		// 真正面
	}
	else if ( aa <= static_cast<float>( M_PI ) )
	{
		// 左回り
		angle_ += direction_fix;

		if ( angle_ > a )
		{
			angle_ = a;
		}
	}
	else if ( aa > static_cast<float>( M_PI ) )
	{
		// 右回り
		angle_ -= direction_fix;

		if ( angle_ < a )
		{
			angle_ = a;
		}
	}
	else
	{
		
	}

	// 角度の正確さ率 0.f .. 1.f
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