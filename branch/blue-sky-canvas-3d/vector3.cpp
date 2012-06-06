//*****************************************************************************
//
//	vector3.cpp
//
//	2002/2/6
//
//*****************************************************************************

#include	"vector3.h"
#include	"matrix4x4.h"
#include	<math.h>
#include	<memory>

vector3::vector3()
{
	set( 0, 0, 0 );
}
vector3::vector3( float a, float b, float c )
{
	set( a, b, c );
}

// to unit vector
void vector3::unit()
{
	float len = float( sqrt( data[0] * data[0] + data[1] * data[1] + data[2] * data[2] ) );
	
	if( len != 0 ){
		len = 1 / len;
		data[0] *= len;
		data[1] *= len;
		data[2] *= len;
	}
}
// get normal vector
void vector3::normal( const vector3 &_v1, const vector3 &_v2, const vector3 &_v3 )
{
	vector3	v1 = _v1;
	vector3	v2 = _v2;
	vector3	v3 = _v3;

	vector	a, b;

	a[0] = v2.get(0) - v1.get(0);
	a[1] = v2.get(1) - v1.get(1);
	a[2] = v2.get(2) - v1.get(2);
	b[0] = v3.get(0) - v1.get(0);
	b[1] = v3.get(1) - v1.get(1);
	b[2] = v3.get(2) - v1.get(2);

	data[0] = a[1] * b[2] - a[2] * b[1];
	data[1] = a[2] * b[0] - a[0] * b[2];
	data[2] = a[0] * b[1] - a[1] * b[0];

	unit();
}

vector3& vector3::operator = ( const vector3 &v )
{
	memcpy( data, v.data, sizeof(float) * 3 );
	return *this;
}

vector3 vector3::operator + ( const vector3 &v ) const
{
	vector3 dv;
	for( int i = 0; i < 3; i++ ){
		dv.data[i] = data[i] + v.data[i];
	}
	return dv;
}

vector3& vector3::operator += ( const vector3 &v )
{
	for( int i = 0; i < 3; i++ ){
		data[i] += v.data[i];
	}
	return *this;
}

vector3 vector3::operator - ( const vector3 &v ) const
{
	vector3 dv;
	for( int i = 0; i < 3; i++ ){
		dv.data[i] = data[i] - v.data[i];
	}
	return dv;
}

vector3& vector3::operator -= ( const vector3 &v )
{
	for( int i = 0; i < 3; i++ ){
		data[i] -= v.data[i];
	}
	return *this;
}

vector3 vector3::operator * ( const matrix4x4 &_x ) const
{
	// | x' |   | a b c d |   | x |
	// | y' | = | e f g h | * | y |
	// | z' |   | i j k l |   | z |
	// | 1  |   | m n o p |   | 1 |

	matrix4x4	x = _x;
	vector3		dv;
	
	const T &sx = data[0], &sy = data[1], &sz = data[2];

	const T &a = x.get(0,0), &b = x.get(0,1), &c = x.get(0,2), &d = x.get(0,3);
	const T &e = x.get(1,0), &f = x.get(1,1), &g = x.get(1,2), &h = x.get(1,3);
	const T &i = x.get(2,0), &j = x.get(2,1), &k = x.get(2,2), &l = x.get(2,3);
	const T &m = x.get(3,0), &n = x.get(3,1), &o = x.get(3,2), &p = x.get(3,3);

	dv.set(
		sx * a + sy * b + sz * c + d,
		sx * e + sy * f + sz * g + h,
		sx * i + sy * j + sz * k + l
	//	sx * m + sy * n + sz * o + p
	);

	return dv;
}

vector3& vector3::operator *= ( const matrix4x4 &x )
{
	*this = operator *( x );
	return *this;
}

void vector3::set( float a, float b, float c )
{
	data[0] = a;
	data[1] = b;
	data[2] = c;
}

void vector3::set( int i, float a )
{
	check_index( i );
	data[i] = a;
}

const float& vector3::get( int i ) const
{
	check_index( i );
	return data[i];
}

const float* vector3::get() const
{
	return data;
}

void vector3::check_index( int i ) const
{
	if( (i < 0) || (i > 2) ){
		throw "vector3 index over";
	}
}


// file access
bool vector3::fwrite( FILE *f )
{
	return ( ::fwrite( data, sizeof(vector), 1, f ) == 1 );
}
bool vector3::fread( FILE *f )
{
	return ( ::fread( data, sizeof(vector), 1, f ) == 1 );
}