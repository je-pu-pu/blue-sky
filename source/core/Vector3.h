//*****************************************************************************
//
//	Vector3.h
//
//	2002/2/6
//
//*****************************************************************************

#ifndef VECTOR_3
#define VECTOR_3

#include <cmath>

class Matrix4x4;

/// @todo 独自実装をやめ内部的に何かのライブラリを利用する
class Vector3
{
public:
	typedef float T;

public:
	Vector3();
	Vector3( T, T = 0, T = 0 );
	~Vector3() { }

	void			init(){ set( 0, 0, 0 ); }
	void			unit();
	void			normal( const Vector3&, const Vector3&, const Vector3& );

	Vector3			operator - () const { return Vector3( -x(), -y(), -z() ); }

	Vector3&		operator =  ( const Vector3& );
	Vector3			operator +  ( const Vector3& ) const;
	Vector3&		operator += ( const Vector3& );
	Vector3			operator -  ( const Vector3& ) const;
	Vector3&		operator -= ( const Vector3& );
	Vector3			operator *  ( const Matrix4x4& ) const;
	Vector3&		operator *= ( const Matrix4x4& );

	Vector3			operator * ( T v ) const { return Vector3( x() * v, y() * v, z() * v ); }
	Vector3			operator / ( T v ) const { return Vector3( x() / v, y() / v, z() / v ); }
	Vector3&		operator *= ( T v ) { set( x() * v, y() * v, z() * v ); return *this; }
	Vector3&		operator /= ( T v ) { set( x() / v, y() / v, z() / v ); return *this; }

	T				length() const { return std::sqrt( x() * x() + y() * y() + z() * z() ); }
	T				length_xy() const { return std::sqrt( x() * x() + y() * y() ); }

	T				dot_product( const Vector3& v ) const { return x() * v.x() + y() * v.y() * z() * v.z(); }
	Vector3			cross_product( const Vector3& v ) const
	{
		return Vector3(
			y() * v.z() - z() * v.y(),
			z() * v.x() - x() * v.z(),
			x() * v.y() - y() * v.x()
		);
	}

	T& x() { return data[ 0 ]; }
	T& y() { return data[ 1 ]; }
	T& z() { return data[ 2 ]; }

	const T& x() const { return data[ 0 ]; }
	const T& y() const { return data[ 1 ]; }
	const T& z() const { return data[ 2 ]; }

	void set_x( T v ) { data[ 0 ] = v; }
	void set_y( T v ) { data[ 1 ] = v; }
	void set_z( T v ) { data[ 2 ] = v; }

	void set( T x, T y, T z )
	{
		set_x( x );
		set_y( y );
		set_z( z );
	}
	
protected:
	typedef T		vector[3];
	vector			data;
};

#endif // VECTOR_3