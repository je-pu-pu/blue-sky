//*****************************************************************************
//
//	Vector3.cpp
//
//	2002/2/6
//
//*****************************************************************************

#include "Vector3.h"
#include "Matrix4x4.h"

#include <math.h>

#include <memory>

Vector3::Vector3()
{
	set( 0, 0, 0 );
}
Vector3::Vector3( float a, float b, float c )
{
	set( a, b, c );
}

// to unit vector
void Vector3::unit()
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
void Vector3::normal( const Vector3 &_v1, const Vector3 &_v2, const Vector3 &_v3 )
{
	Vector3	v1 = _v1;
	Vector3	v2 = _v2;
	Vector3	v3 = _v3;

	vector	a, b;

	a[0] = v2.data[ 0 ] - v1.data[ 0 ];
	a[1] = v2.data[ 1 ] - v1.data[ 1 ];
	a[2] = v2.data[ 2 ] - v1.data[ 2 ];
	b[0] = v3.data[ 0 ] - v1.data[ 0 ];
	b[1] = v3.data[ 1 ] - v1.data[ 1 ];
	b[2] = v3.data[ 2 ] - v1.data[ 2 ];

	data[0] = a[1] * b[2] - a[2] * b[1];
	data[1] = a[2] * b[0] - a[0] * b[2];
	data[2] = a[0] * b[1] - a[1] * b[0];

	unit();
}

Vector3& Vector3::operator = ( const Vector3 &v )
{
	memcpy( data, v.data, sizeof(float) * 3 );
	return *this;
}

Vector3 Vector3::operator + ( const Vector3 &v ) const
{
	Vector3 dv;
	for( int i = 0; i < 3; i++ ){
		dv.data[i] = data[i] + v.data[i];
	}
	return dv;
}

Vector3& Vector3::operator += ( const Vector3 &v )
{
	for( int i = 0; i < 3; i++ ){
		data[i] += v.data[i];
	}
	return *this;
}

Vector3 Vector3::operator - ( const Vector3 &v ) const
{
	Vector3 dv;
	for( int i = 0; i < 3; i++ ){
		dv.data[i] = data[i] - v.data[i];
	}
	return dv;
}

Vector3& Vector3::operator -= ( const Vector3 &v )
{
	for( int i = 0; i < 3; i++ ){
		data[i] -= v.data[i];
	}
	return *this;
}

Vector3 Vector3::operator * ( const Matrix4x4 &_x ) const
{
	// | x' |   | a b c d |   | x |
	// | y' | = | e f g h | * | y |
	// | z' |   | i j k l |   | z |
	// | 1  |   | m n o p |   | 1 |

	Matrix4x4	x = _x;
	Vector3		dv;
	
	const T &sx = data[0], &sy = data[1], &sz = data[2];

	const T &a = x.get(0,0), &b = x.get(0,1), &c = x.get(0,2), &d = x.get(0,3);
	const T &e = x.get(1,0), &f = x.get(1,1), &g = x.get(1,2), &h = x.get(1,3);
	const T &i = x.get(2,0), &j = x.get(2,1), &k = x.get(2,2), &l = x.get(2,3);
//	const T &m = x.get(3,0), &n = x.get(3,1), &o = x.get(3,2), &p = x.get(3,3);

	dv.set(
		sx * a + sy * b + sz * c + d,
		sx * e + sy * f + sz * g + h,
		sx * i + sy * j + sz * k + l
	//	sx * m + sy * n + sz * o + p
	);

	return dv;
}

Vector3& Vector3::operator *= ( const Matrix4x4 &x )
{
	*this = operator *( x );
	return *this;
}
