//*****************************************************************************
//
//	matrix4x4.cpp
//
//	2002/3/16
//
//*****************************************************************************

#include	"matrix4x4.h"
#include	"vector3.h"
#include	<memory>
#include	<math.h>

static const double pi = 3.1415926535897932384626433832795028841971693993751058209749445923078164;

matrix4x4::matrix4x4()
{
	init();
}

matrix4x4::matrix4x4(	float a, float b, float c, float d,
						float e, float f, float g, float h,
						float i, float j, float k, float l,
						float m, float n, float o, float p )
{
	set( a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p );
}

void matrix4x4::init()
{
	// | 1 0 0 0 |
	// | 0 1 0 0 |
	// | 0 0 1 0 |
	// | 0 0 0 1 |
	data[0][0] = 1; data[0][1] = 0; data[0][2] = 0; data[0][3] = 0;
	data[1][0] = 0; data[1][1] = 1; data[1][2] = 0; data[1][3] = 0;
	data[2][0] = 0; data[2][1] = 0; data[2][2] = 1; data[2][3] = 0;
	data[3][0] = 0; data[3][1] = 0; data[3][2] = 0; data[3][3] = 1;
}

// data copy
matrix4x4& matrix4x4::operator =( const matrix4x4 &m )
{
	memcpy( data, m.data, sizeof(float) * 4 * 4 );
	return *this;
}

// multi matrix
matrix4x4 matrix4x4::operator *( const matrix4x4 &m )
{
	matrix4x4 ret;
	
	for( int i = 0; i < 4; i++ ){
		for( int j = 0; j < 4; j++ ){
			float d = 0;
			for( int k = 0; k < 4; k++ ){
				d += data[i][k] * m.data[k][j];
			}
			ret.data[i][j] = d;
		}
	}
	return ret;
}
matrix4x4& matrix4x4::operator *=( const matrix4x4 &_m )
{
	matrix4x4 m = _m;
	*this = m * ( *this );
	return *this;
}

void matrix4x4::set(
						  float a, float e, float i, float m,
						  float b, float f, float j, float n,
						  float c, float g, float k, float o,
						  float d, float h, float l, float p )
{
	// | a e i m |
	// | b f j n |
	// | c g k o |
	// | d h l p |
	data[0][0] = a; data[0][1] = e; data[0][2] = i; data[0][3] = m;
	data[1][0] = b; data[1][1] = f; data[1][2] = j; data[1][3] = n;
	data[2][0] = c; data[2][1] = g; data[2][2] = k; data[2][3] = o;
	data[3][0] = d; data[3][1] = h; data[3][2] = l; data[3][3] = p;
}

void matrix4x4::set( int y, int x, float v )
{
	check_index( y, x );
	data[y][x] = v;
}

const float& matrix4x4::get( int y, int x )
{
	check_index( y, x );
	return data[y][x];
}

void matrix4x4::translate( float x, float y, float z )
{
	matrix4x4 m(
		1, 0, 0, x,
		0, 1, 0, y,
		0, 0, 1, z,
		0, 0, 0, 1
	);
	operator *=( m );
}

void matrix4x4::translate( const vector3 &_v )
{
	vector3 v = _v;
	translate( v.x(), v.y(), v.z() );
}

void matrix4x4::scale( float x, float y, float z )
{
	matrix4x4 m(
		x, 0, 0, 0,
		0, y, 0, 0,
		0, 0, z, 0,
		0, 0, 0, 1
	);
	operator *=( m );
}

void matrix4x4::rotate_axis( float a, float x, float y, float z )
{
	// x*x*(1-cos(r))+cos(r)	x*y*(1-cos(r))-z*sin(r)	x*z*(1-cos(r))+y*sin(r)
	// y*x*(1-cos(r))+z*sin(r)	y*y*(1-cos(r))+cos(r)	y*z*(1-cos(r))-x*sin(r)
	// z*x*(1-cos(r))-y*sin(r)	z*y*(1-cos(r))+x*sin(r)	z*z*(1-cos(r))+cos(r)

	// | a b c d |
	// | e f g h |
	// | i j k l |
	// | m n o p |

	// if( (x != 0) && (y == 0) && (z == 0) )	rotate

	// unit
	vector3 v( x, y, z );
	v.unit();
	x = v.x();
	y = v.y();
	z = v.z();

	a *= static_cast< float >( pi / 180.0 );
	float sa = sin( a );
	float ca = cos( a );

	matrix4x4 m(
		x * x * (1 - ca) + ca,		// a
		x * y * (1 - ca) - z * sa,	// b
		x * z * (1 - ca) + y * sa,	// c
		0,							// d
		y * x * (1 - ca) + z * sa,	// e
		y * y * (1 - ca) + ca,		// f
		y * z * (1 - ca) - x * sa,	// g
		0,							// h
		z * x * (1 - ca) - y * sa,	// i
		z * y * (1 - ca) + x * sa,	// j
		z * z * (1 - ca) + ca,		// k
		0,							// l
		0, 0, 0, 1					// m n o p
	);

	operator *=( m );
}

void matrix4x4::rotate_axis( float a, const vector3 &_v )
{
	vector3 v = _v;
	rotate_axis( a, v.x(), v.y(), v.z() );
}

// rotate with x axis
void matrix4x4::rotate_x( float a )
{
	// |     1     0     0     0     |
	// |     0  cosX -sinX     0     |
	// |     0  sinX  cosX     0     |
	// |     0     0     0     1     |

	a *= static_cast< float >( pi / 180.0 );
	float sa = sin( a );
	float ca = cos( a );

	matrix4x4 m(
		1,   0,   0,   0,
		0,  ca, -sa,   0,
		0,  sa,  ca,   0,
		0,   0,   0,   1
	);
	operator *=( m );
}

// rotate with y axis
void matrix4x4::rotate_y( float a )
{
	// |  cosY     0  sinY     0     |
	// |     0     1     0     0     |
	// | -sinY     0  cosY     0     |
	// |     0     0     0     1     |

	a *= static_cast< float >( pi / 180.0 );
	float sa = sin( a );
	float ca = cos( a );

	matrix4x4 m(
		 ca,   0,  sa,   0,
		  0,   1,   0,   0,
		-sa,   0,  ca,   0,
		  0,   0,   0,   1
	);
	operator *=( m );
}

// rotate with z axis
void matrix4x4::rotate_z( float a )
{	
	// |  cosZ -sinZ     0     0     |
	// |  sinZ  cosZ     0     0     |
	// |     0     0     1     0     |
	// |     0     0     0     1     |

	a *= static_cast< float >( pi / 180.0 );
	float sa = sin( a );
	float ca = cos( a );

	matrix4x4 m(
		 ca, -sa,   0,   0,
		 sa,  ca,   0,   0,
		  0,   0,   1,   0,
		  0,   0,   0,   1
	);
	operator *=( m );
}

void matrix4x4::check_index( int y, int x )
{
	if( y < 0 )	throw "matrix4x4 y index over";
	if( y > 3 )	throw "matrix4x4 y index over";
	if( x < 0 )	throw "matrix4x4 x index over";
	if( x > 3 )	throw "matrix4x4 x index over";
}
