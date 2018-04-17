//*****************************************************************************
//
//	Matrix4x4.h
//
//	2002/2/4
//
//*****************************************************************************

#ifndef _matrix4x4_
#define _matrix4x4_

// uses
class Vector3;

class Matrix4x4
{
public:
	Matrix4x4();
	Matrix4x4(
		float, float, float, float,
		float, float, float, float,
		float, float, float, float,
		float, float, float, float );
	virtual ~Matrix4x4(){};

	void init();
	void translate( float, float, float );
	void translate( const Vector3& );
	void rotate_axis( float, float, float, float );
	void rotate_axis( float, const Vector3& );
	void rotate_x( float );
	void rotate_y( float );
	void rotate_z( float );
	void scale( float, float, float );

	Matrix4x4&	operator =( const Matrix4x4& );
	Matrix4x4	operator *( const Matrix4x4& );
	Matrix4x4&	operator *=( const Matrix4x4& );

	void	set(
		float, float, float, float,
		float, float, float, float,
		float, float, float, float,
		float, float, float, float );
	void			set( int, int, float );
	const float&	get( int, int );
	const float*	get(){ return data[0]; }

private:
	typedef float matrix4[4][4];

	matrix4		data;

	void		check_index( int, int );
};

#endif