#pragma once

#include "Vector.h"

namespace direct_x_math
{

/**
 * “à•”“I‚É DirectXMath ‚ðŽg—p‚µ‚½ Matrix
 *
 */
class alignas( 16 ) Matrix
{
public:
	typedef float			UnitType;
	typedef int				IntType;
	typedef unsigned int	UintType;

private:
	DirectX::XMMATRIX value_;

public:
	Matrix()
	{
		
	}

	Matrix( const DirectX::XMMATRIX& m )
	{
		value_ = m;
	}

#if 0
	Matrix( DirectX::XMFLOAT4X4 m )
	{
		value_ = DirectX::XMLoadFloat4x4( & m );
	}
#endif

	Matrix& operator = ( const Matrix& m )
	{
		value_ = m.value_;

		return *this;
	}
	
	/*
	UnitType get( int y, int x )
	{
		return value_.m[ y ][ x ];
	}
	*/

	Matrix& set(
		UnitType m00, UnitType m01, UnitType m02, UnitType m03,
		UnitType m10, UnitType m11, UnitType m12, UnitType m13,
		UnitType m20, UnitType m21, UnitType m22, UnitType m23,
		UnitType m30, UnitType m31, UnitType m32, UnitType m33 )
	{
		value_ = DirectX::XMMATRIX(
			m00, m01, m02, m03,
			m10, m11, m12, m13,
			m20, m21, m22, m23,
			m30, m31, m32, m33
		);
		
		return *this;
	}

	Matrix& set_identity()
	{
		value_ = DirectX::XMMatrixIdentity();

		return *this;
	}

	Matrix& set_translation( UnitType tx, UnitType ty, UnitType tz )
	{
		value_ = DirectX::XMMatrixTranslation( tx, ty, tz );

		return *this;
	}

	Matrix& add_translation( UnitType tx, UnitType ty, UnitType tz )
	{
		value_ *= DirectX::XMMatrixTranslation( tx, ty, tz );

		return *this;
	}

	Matrix& set_rotation_x( UnitType r )
	{
		value_ = DirectX::XMMatrixRotationX( r );

		return *this;
	}

	Matrix& set_rotation_y( UnitType r )
	{
		value_ = DirectX::XMMatrixRotationY( r );

		return *this;
	}

	Matrix& set_rotation_z( UnitType r )
	{
		value_ = DirectX::XMMatrixRotationZ( r );

		return *this;
	}

	Matrix& set_rotation_xyz( UnitType rx, UnitType ry, UnitType rz )
	{
		value_ = DirectX::XMMatrixRotationX( rx ) * DirectX::XMMatrixRotationY( ry ) * DirectX::XMMatrixRotationZ( rz );

		return *this;
	}

	Matrix& set_rotation_roll_pitch_yaw( UnitType rx, UnitType ry, UnitType rz )
	{
		value_ = DirectX::XMMatrixRotationRollPitchYaw( rx, ry, rz );

		return *this;
	}

	Matrix& set_rotation_quaternion( const Vector& q )
	{
		value_ = DirectX::XMMatrixRotationQuaternion( q );

		return *this;
	}

	Matrix& set_scaling( UnitType sx, UnitType sy, UnitType sz )
	{
		value_ = DirectX::XMMatrixScaling( sx, sy, sz );

		return *this;
	}

	Matrix& set_look_at( const Vector& eye, const Vector& at, const Vector& up )
	{
		value_ = DirectX::XMMatrixLookAtLH( eye, at, up );

		return *this;
	}

	Matrix inverse() const
	{
		DirectX::XMVECTOR v;
		return Matrix( XMMatrixInverse( & v, value_ ) );
	}

	Matrix transpose() const
	{
		return Matrix( XMMatrixTranspose( value_ ) );
	}

	Matrix& set_perspective_fov( UnitType fov_angle_y, UnitType aspect_ratio, UnitType near_z, UnitType far_z )
	{
		value_ = DirectX::XMMatrixPerspectiveFovLH( fov_angle_y, aspect_ratio, near_z, far_z );

		return *this;
	}

	Matrix& set_orthographic( UnitType w, UnitType h, UnitType near_z, UnitType far_z )
	{
		value_ = DirectX::XMMatrixOrthographicLH( w, h, near_z, far_z );

		return *this;
	}

	Matrix& operator *= ( const Matrix& m )
	{
		value_ *= m.value_;

		return *this;
	}

	Matrix operator * ( const Matrix& m ) const
	{
		return Matrix( value_ * m.value_ );
	}

	friend Vector operator * ( const Vector& v, const Matrix& m )
	{
		return XMVector4Transform( v.value_, m.value_ );
	}

	friend Vector& operator *= ( Vector& v, const Matrix& m )
	{
		v = v * m;

		return v;
	}

	operator DirectX::XMMATRIX () const { return value_; }


	static Matrix identity()
	{
		Matrix m;
		m.set_identity();

		return m;
	}

	static void* operator new ( size_t size )
	{
        return _aligned_malloc( size, 16 );
    }

	static void operator delete ( void* p )
	{
		_aligned_free( p );
	}

}; // class Matrix

}; // namespace direct_x_math
