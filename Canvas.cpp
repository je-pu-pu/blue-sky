#include "Canvas.h"

#include <common/Random.h>
#include <common/math.h>

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

void Canvas::clear()
{
	vertex_list().clear();
	line_list().clear();
	face_list().clear();
}

/**
 * �f�v�X�o�b�t�@���쐬����
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
 * �菑�����̐���`�悷��
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
	
	const Real edge_pos_random = 0.2f;

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

	// ��������
	if ( g_solid )
	{
		color.a() = 200;
	}

	// �ʔ��݂̖����G��
	if ( g_line )
	{
		return drawLine( v1.x(), v1.y(), v2.x(), v2.y(), color );
	}

	int division = 1000; // ��{�̐���`�悷�邽�߂ɍő剽�̉~��`�悷�邩
	
	const Real min_interval = 1.f;		// �~�Ɖ~�̊Ԋu�̍ŏ��l ( pixel )
	const Real max_interval = 8.f;		// �~�Ɖ~�̊Ԋu�̍ő�l ( pixel )

	Real power = 1.2f; // �M�� ( pixel )
	Real power_min = 1.f; // �Œ�M�� ( pixel )
	Real power_max = 20.f; // �ő�M�� ( pixel )
	Real power_plus = 0.02f; //
	Real power_reset = 0.8f; // �M�����Z�b�g���̔{��
	Real power_plus_reset = -1.f;

	Real direction = atan2( v2.y() - v1.y(), v2.x() - v1.x() ); // ���݂̕M�̈ʒu����I���_�܂ł̐��m�ȕ��� ( radian )
	Real direction_fix_default = 0.0001f; // �M�̕����̕␳�l�̍ŏ��l ( radian )
	Real direction_fix = direction_fix_default; // �M�̕����̕␳�l ( radian )
	Real direction_fix_acceleration = 0.01f; // �M�̕����̕␳�l�̉����x ( radian )
	Real direction_random = 0.01f; // �M�̕����̃����_����

	int color_plus = -2; // �F�̕ω�

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

	Real a = 0.f; // �M���̃A��
	const Real random_direction = common::random( -direction_random, direction_random );

	// Real d = direction + ( ( rand() % 100 / 100.f ) - 0.5f ) * direction_random; // ���݂̕M�̕��� ( radian )
	
	Real d = direction + direction_random; // ���݂̕M�̕��� ( radian )

	// direction = d;

	const Real dz = ( v2.z() - v1.z() ) / division;

	for ( int n = 0; n < division; n++ )
	{
		// �~��`�悷��
		if ( depthTest( v1 ) )
		{
			drawCircle( v1, power, color, true );
			g_circle_count++;
		}
		else
		{
			power *= 0.9f;
		}

		Real interval = std::min( 1.f, max_interval );
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

		// ���݂̍��W���� to �ւ̐��m�Ȋp�x
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

		// ���݂̊p�x�𐳊m�Ȋp�x�֋ߕt����
		if ( dd == 0 )
		{
			// ����
		}
		else if ( dd <= static_cast< float >( M_PI ) )
		{
			// �����
			d += direction_fix;
		}
		else if ( dd > static_cast< float >( M_PI ) )
		{
			// �E���
			d -= direction_fix;
		}
		else
		{
			break;
		}

		// to �܂ŕ`�悵����I������
		if ( len < interval )
		{
			// �~��`�悷��
			if ( depthTest( v1 ) )
			{
				drawCircle( to, power, color, true );
				g_circle_count++;
			}

			break;
		}

		direction_fix += direction_fix_acceleration;
		d += random_direction * 0.1f;

		color.r() = math::clamp( color.r() + common::random( -2, 1 ), 0, 255 );
		color.g() = math::clamp( color.g() + common::random( -2, 1 ), 0, 255 );
		color.b() = math::clamp( color.b() + common::random( -2, 1 ), 0, 255 );
		color.a() = math::clamp( color.a() - 1, 0, 255 );
	}
}

void Canvas::drawLineHumanTouch( Real x1, Real y1, Real x2, Real y2, const Color& color )
{
	drawLineHumanTouch( art::Vertex( x1, y1 ), art::Vertex( x2, y2 ), color );
}

void Canvas::drawPolygonHumanTouch( const Face& face, const Color& c )
{
	if ( face.id_list().size() < 4 )
	{
		return;
	}

	// quad
	{
		const Real z_offset = 0.001f;

		art::Vertex from = vertex_list()[ face.id_list()[ 0 ] ].vertex();
		art::Vertex to = vertex_list()[ face.id_list()[ 1 ] ].vertex();

		from.z() += z_offset;
		to.z() += z_offset;

		art::Vertex from2 = vertex_list()[ face.id_list()[ 3 ] ].vertex();
		art::Vertex to2 = vertex_list()[ face.id_list()[ 2 ] ].vertex();

		from2.z() += z_offset;
		to2.z() += z_offset;

		art::Vertex::T div = ( from2 - from ).length_xy() / ( ( brush_ ? brush_->size() : 4.f ) * 1.2f );

		art::Vertex from_inc = ( from2 - from ) / div;
		art::Vertex to_inc   = ( to2 - to ) / div;

		from_inc.z() = 0.f;
		to_inc.z() = 0.f;

		from.z() = ( from.z() + to.z() + from2.z() + to2.z() ) / 4.f;
		to.z() = from.z();

		art::Color color = c;

		for ( int n = 0; n < div - 1; n++ )
		{
			drawLineHumanTouch( from, to, color );
			
			color.r() = math::clamp( color.r() + common::random( -1, 1 ), 0, 255 );
			color.g() = math::clamp( color.g() + common::random( -1, 1 ), 0, 255 );
			color.b() = math::clamp( color.b() + common::random( -1, 1 ), 0, 255 );

			from += from_inc;
			to += to_inc;
		}
	}
}

bool Canvas::depthTest( const art::Vertex& v )
{
	// return true;

	const int x = static_cast< int >( v.x() ) / DEPTH_BUFFER_PIXEL_SIZE;
	const int y = static_cast< int >( v.y() ) / DEPTH_BUFFER_PIXEL_SIZE;;

	if ( x < 0 ) return true;
	if ( x >= depth_buffer_width_ ) return true;
	if ( y < 0 ) return true;
	if ( y >= depth_buffer_height_ ) return true;

	const int index = y * depth_buffer_width_ + x;

	if ( depth_buffer_pixel_id_ == depth_buffer_[ index ].second )
	{
		depth_buffer_[ index ].first = std::min( depth_buffer_[ index ].first, v.z() );
		return true; // depth_buffer_last_test_;
	}

	if ( v.z() < depth_buffer_[ index ].first )
	{
		depth_buffer_[ index ] = std::pair< Real, art::ID >( v.z(), depth_buffer_pixel_id_ );
		depth_buffer_last_index_ = index;
		depth_buffer_last_test_ = true;

		return true;
	}

	depth_buffer_last_test_ = false;

	return false;
}

void Canvas::sort_face_list_by_z()
{
	std::sort( face_list().begin(), face_list().end(), face_z_compare( this ) );
}

bool Canvas::face_z_compare::operator () ( const Face& a, const Face& b )
{
	Real az = 0.f;
	Real bz = 0.f;

	for ( Face::IDList::const_iterator i = a.id_list().begin(); i != a.id_list().end(); ++i )
	{
		az += canvas_->vertex_list_[ *i ].vertex().z();
	}

	for ( Face::IDList::const_iterator i = b.id_list().begin(); i != b.id_list().end(); ++i )
	{
		bz += canvas_->vertex_list_[ *i ].vertex().z();
	}

	return az > bz;
}

void Canvas::sort_line_list_by_z()
{
	std::sort( line_list().begin(), line_list().end(), line_z_compare( this ) );
}

bool Canvas::line_z_compare::operator () ( const Line& a, const Line& b )
{
	return ( canvas_->vertex_list_[ a.from() ].vertex().z() + canvas_->vertex_list_[ a.to() ].vertex().z() ) < ( canvas_->vertex_list_[ b.from() ].vertex().z() + canvas_->vertex_list_[ b.to() ].vertex().z() );
}

Canvas::Vertex::Vertex()
	: angle_( 0.f )
	, speed_( 0.f )
{
	//
}

void Canvas::Vertex::update()
{
	// vertex() = target_vertex();
	// vertex() = ( vertex() + target_vertex() ) / 2.f;
	// return;
		
	// static float a = 0.f;
	// a += 0.01f;

	const Real rs = 4.f;
	art::Vertex d = target_vertex() - vertex();
	art::Vertex r( common::random( -rs, rs ), common::random( -rs, rs ) );

	direction() = ( d * 1 + r * 1 ) / 2;
	vertex_ += direction_;
	return;

	// direction() *= ( sin( a ) + 1.f ) * 0.1f;
	// vertex_ += direction_;


	/// ����̑��x
	// const float turn_speed = 5.f;

	const float rx = target_vertex().x() - vertex().x();
	const float ry = target_vertex().y() - vertex().y();
	
	const float len = sqrt( rx * rx + ry * ry );
	float a = atan2( ry, rx );
	
	/// ���_�̍ő�ړ����x
	const float max_speed = len;
	const float min_speed = len / 8.f;

	/// �����x
	const float speed_fix = 8.f;

	/// ���񑬓x
	const float direction_fix = 0.2f;

	// 0 <= a < 2 * M_PI �Ɋۂ߂�
	while ( a < 0.f )
	{
		a += 2.f * static_cast<float>( M_PI );
	}
	while ( a >= 2.f * static_cast<float>( M_PI ) )
	{
		a -= 2.f * static_cast<float>( M_PI ) ;
	}

	// 0 <= angle_ < 2 * M_PI �Ɋۂ߂�
	while ( angle_ < 0.f )
	{
		angle_ += 2.f * static_cast<float>( M_PI );
	}
	while ( angle_ >= 2.f * static_cast<float>( M_PI ) )
	{
		angle_ -= 2.f * static_cast<float>( M_PI );
	}

	float aa = a - angle_;

	// 0 <= aa < 2 * M_PI �Ɋۂ߂�
	while ( aa < 0 )
	{
		aa += ( 2 * static_cast<float>( M_PI ) );
	}

	if ( aa == 0.f )
	{
		// �^����
	}
	else if ( aa <= static_cast<float>( M_PI ) )
	{
		// �����
		angle_ += direction_fix;

		if ( angle_ > a )
		{
			angle_ = a;
		}
	}
	else if ( aa > static_cast<float>( M_PI ) )
	{
		// �E���
		angle_ -= direction_fix;

		if ( angle_ < a )
		{
			angle_ = a;
		}
	}
	else
	{
		
	}

	// angle_ = a;

	// �p�x�̐��m���� 0.f .. 1.f
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

	const float speed = std::min( max_speed, len ) * ( pow( angle_correct_rate, 8 ) );
	// const float speed = len / 4.f; // * pow( angle_correct_rate, 8 );

	// const float speed = min( max_speed, len );

	if ( speed_ < speed )
	{
		speed_ *= 1.8f; // speed_fix;
	}
	else if ( speed_ > speed )
	{
		speed_ /= 1.8f; // speed_fix;

		if ( speed_ < speed )
		{
			speed_ = speed;
		}
	}

	if ( speed_ < min_speed )
	{
		speed_ = min_speed;
	}
	if ( speed_ > speed )
	{
		speed_ = speed;
	}
	
	// speed_ = speed;

	// speed_ = len / 2;

	direction_.x() = cos( angle_ ) * speed_; // min( diff * 0.1f, len );
	direction_.y() = sin( angle_ ) * speed_; // min( diff * 0.1f, len );

	// direction() = ( direction() * 15 + d * 1 ) / 16;

	vertex_ += direction_;
	vertex_.z() = target_vertex().z();
}

}; // namespace art