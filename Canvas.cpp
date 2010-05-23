#include "Canvas.h"

#include <common/Random.h>

#include <math.h>
#include <assert.h>

bool g_solid = false;
bool g_line = false;

float g_power = 1.f;
float g_power_min = 1.f;
float g_power_max = 32.f;
float g_power_plus = 0.1f;
float g_power_rest = 0.8f;
float g_power_plus_reset = -1.f;

float g_direction_fix_default = 0.0001f;
float g_direction_fix_acceleration = 0.00001f;
float g_direction_random = 0.f;

int g_line_count = 0;
int g_circle_count = 0;

namespace art
{

Canvas::Canvas()
	: brush_( 0 )
	, depth_buffer_( 0 )
	, depth_buffer_last_index_( -1 )
{
	
}

Canvas::~Canvas()
{
	delete depth_buffer_;
}

/**
 * デプスバッファを作成する
 *
 */
void Canvas::createDepthBuffer()
{
	depth_buffer_width_ = width() / DEPTH_BUFFER_PIXEL_SIZE;
	depth_buffer_height_ = height() / DEPTH_BUFFER_PIXEL_SIZE;

	depth_buffer_ = new std::pair< Real, art::ID >[ depth_buffer_width_ * depth_buffer_height_ ];

	clearDepthBuffer();
}

void Canvas::clearDepthBuffer()
{
	for ( int n = 0; n < depth_buffer_width_ * depth_buffer_height_; n++ )
	{
		depth_buffer_[ n ] = std::pair< Real, art::ID >( 9999.f, -1 );
	}
}

void Canvas::clearDepthBufferLastIndex()
{
	depth_buffer_last_index_ = -1;
}

/**
 * 手書き風の線を描画する
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
	
	const Real edge_pos_random = 0.f;

	Color color = c;

	/*
	Real x1 = from.x() + common::random( -edge_pos_random, edge_pos_random );
	Real y1 = from.y() + common::random( -edge_pos_random, edge_pos_random );
	Real x2 = to.x() + common::random( -edge_pos_random, edge_pos_random );
	Real y2 = to.y() + common::random( -edge_pos_random, edge_pos_random );
	*/

	art::Vertex v1 = from + art::Vertex( common::random( -edge_pos_random, edge_pos_random ), common::random( -edge_pos_random, edge_pos_random ) );
	art::Vertex v2 = to + art::Vertex( common::random( -edge_pos_random, edge_pos_random ), common::random( -edge_pos_random, edge_pos_random ) );

	v1.z() = ( from.z() + to.z() ) / 2.f;

	// int r = rand() % 255;
	// RGBQUAD black = { 0, r, r, 20 };

	// くっきり
	if ( g_solid )
	{
		color.a() = 255;
	}

	// 面白みの無い絵に
	if ( g_line )
	{
		// return drawLine( x, y1, x2, y2, c );
	}

	int division = 1000; // 一本の線を描画するために最大何個の円を描画するか
	
	const Real min_interval = 1.f;		// 円と円の間隔の最小値 ( pixel )
	const Real max_interval = 8.f;		// 円と円の間隔の最大値 ( pixel )

	Real power = 1.2f; // 筆圧 ( pixel )
	Real power_min = 1.f; // 最低筆圧 ( pixel )
	Real power_max = 20.f; // 最大筆圧 ( pixel )
	Real power_plus = 0.02f; //
	Real power_reset = 0.8f; // 筆圧リセット時の倍率
	Real power_plus_reset = -1.f;

	Real direction = atan2( v2.y() - v1.y(), v2.x() - v1.x() ); // 現在の筆の位置から終了点までの正確な方向 ( radian )
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

	if ( brush_ )
	{
		power = brush_->size();
		power_plus = brush_->size_acceleration();
	}

	Real a = 0.f; // 筆圧のアレ
	const Real random_direction = common::random( -direction_random, direction_random );

	// Real d = direction + ( ( rand() % 100 / 100.f ) - 0.5f ) * direction_random; // 現在の筆の方向 ( radian )
	
	Real d = direction + direction_random; // 現在の筆の方向 ( radian )

	// direction = d;

	const Real dz = ( v2.z() - v1.z() ) / division;

	for ( int n = 0; n < division; n++ )
	{
		// 円を描画する
		if ( depthTest( v1 ) )
		{
			drawCircle( v1, power, color, true );
			g_circle_count++;
		}
		else
		{
			// power -= 0.1f;
		}

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

		// 現在の座標から to への正確な角度
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

		// 現在の角度を正確な角度へ近付ける
		if ( dd == 0 )
		{
			// 正面
		}
		else if ( dd <= static_cast< float >( M_PI ) )
		{
			// 左回り
			d += direction_fix;
		}
		else if ( dd > static_cast< float >( M_PI ) )
		{
			// 右回り
			d -= direction_fix;
		}
		else
		{
			break;
		}

		// to まで描画したら終了する
		if ( len < interval )
		{
			// 円を描画する
			if ( depthTest( v1 ) )
			{
				drawCircle( to, power, color, true );
				g_circle_count++;
			}

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

bool Canvas::depthTest( const art::Vertex& v )
{
	const int x = static_cast< int >( v.x() ) / DEPTH_BUFFER_PIXEL_SIZE;
	const int y = static_cast< int >( v.y() ) / DEPTH_BUFFER_PIXEL_SIZE;;

	if ( x < 0 ) return false;
	if ( x >= depth_buffer_width_ ) return false;
	if ( y < 0 ) return false;
	if ( y >= depth_buffer_height_ ) return false;

	const int index = y * depth_buffer_width_ + x;

	if ( depth_buffer_pixel_id_ == depth_buffer_[ index ].second )
	{
		depth_buffer_[ index ].first = min( depth_buffer_[ index ].first, v.z() );
		return true; // depth_buffer_last_test_;
	}

	if ( v.z() <= depth_buffer_[ index ].first )
	{
		depth_buffer_[ index ] = std::pair< Real, art::ID >( v.z(), depth_buffer_pixel_id_ );
		depth_buffer_last_index_ = index;
		depth_buffer_last_test_ = true;

		return true;
	}

	depth_buffer_last_test_ = false;

	return false;
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

	/*
	art::Vertex d = target_vertex() - vertex();
	art::Vertex r( rand() % 100 / 100.f * 2.f - 1.f, rand() % 100 / 100.f * 2.f - 1.f, rand() % 100 / 100.f * 2.f - 1.f );

	direction() = ( direction() * 14 + d * 1 + r * 1 ) / 16;
	vertex_ += direction_;
	return;
	*/

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