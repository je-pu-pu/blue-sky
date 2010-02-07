//*****************************************************************************
//
//	vector3.h
//
//	2002/2/6
//
//*****************************************************************************

#ifndef _vector3_
#define _vector3_

#include	<stdio.h>

// uses
class matrix4x4;

class vector3
{
public:
	typedef float T;

public:
	vector3();
	vector3( T, T = 0, T = 0 );
	~vector3() { }

	void			init(){ set( 0, 0, 0 ); }
	void			unit();
	void			normal( const vector3&, const vector3&, const vector3& );

	vector3&		operator =( const vector3& );
	vector3			operator +( const vector3& );
	vector3&		operator +=( const vector3& );
	vector3			operator -( const vector3& );
	vector3&		operator -=( const vector3& );
	vector3			operator *( const matrix4x4& );
	vector3&		operator *=( const matrix4x4& );

	vector3			operator * ( T v ) { return vector3( x() * v, y() * v, z() * v ); }
	vector3			operator / ( T v ) { return vector3( x() / v, y() / v, z() / v ); }
	vector3&		operator *= ( T v ) { set( x() * v, y() * v, z() * v ); return *this; }
	vector3&		operator /= ( T v ) { set( x() / v, y() / v, z() / v ); return *this; }

	T				dot_product( const vector3& v ) const { return x() * v.x() + y() * v.y() * z() * v.z(); }
	vector3			cross_product( const vector3& v ) const
	{
		return vector3(
			y() * v.z() - z() * v.y(),
			z() * v.x() - x() * v.z(),
			x() * v.y() - y() * v.x()
		);
	}

	void			set( T, T, T );
	void			set( int, T );
	
	const T&		get( int ) const;
	const T*		get() const;
	
	
	T&				x() { return data[ 0 ]; }
	T&				y() { return data[ 1 ]; }
	T&				z() { return data[ 2 ]; }

	const T&		x() const { return get(0); }
	const T&		y() const { return get(1); }
	const T&		z() const { return get(2); }
	

	bool			fwrite( FILE* );
	bool			fread( FILE* );

protected:
	typedef T		vector[3];
	vector			data;

	void			check_index( int ) const;
};

#endif