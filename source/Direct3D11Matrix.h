#ifndef DIRECT_3D_11_MATRIX_H
#define DIRECT_3D_11_MATRIX_H

#include <d3d11.h>
#include <xnamath.h>

#include "Direct3D11Vector.h"

/**
 * Direct3D 11 Matrix
 *
 */
__declspec( align( 16 ) ) class Direct3D11Matrix
{
public:
	typedef float			UnitType;
	typedef int				IntType;
	typedef unsigned int	UintType;

	typedef Direct3D11Vector	VectorType;

private:
	__declspec( align( 16 ) ) XMMATRIX value_;

public:
	Direct3D11Matrix()
	{
		
	}

	Direct3D11Matrix( const XMMATRIX& m )
	{
		value_ = m;
	}

	Direct3D11Matrix( XMFLOAT4X4 m )
	{
		value_ = XMLoadFloat4x4( & m );
	}

	Direct3D11Matrix& operator = ( const Direct3D11Matrix& m )
	{
		value_ = m.value_;

		return *this;
	}
	
	UnitType get( int y, int x )
	{
		return value_.m[ y ][ x ];
	}

	Direct3D11Matrix& set(
		UnitType m00, UnitType m01, UnitType m02, UnitType m03,
		UnitType m10, UnitType m11, UnitType m12, UnitType m13,
		UnitType m20, UnitType m21, UnitType m22, UnitType m23,
		UnitType m30, UnitType m31, UnitType m32, UnitType m33 )
	{
		value_._11 = m00; value_._12 = m01; value_._13 = m02; value_._14 = m03;
		value_._21 = m10; value_._22 = m11; value_._23 = m12; value_._24 = m13;
		value_._31 = m20; value_._32 = m21; value_._33 = m22; value_._34 = m23;
		value_._41 = m30; value_._42 = m31; value_._43 = m32; value_._44 = m33;

		return *this;
	}

	Direct3D11Matrix& set_identity()
	{
		value_ = XMMatrixIdentity();

		return *this;
	}

	Direct3D11Matrix& set_translation( UnitType tx, UnitType ty, UnitType tz )
	{
		value_ = XMMatrixTranslation( tx, ty, tz );

		return *this;
	}

	Direct3D11Matrix& add_translation( UnitType tx, UnitType ty, UnitType tz )
	{
		value_._41 += tx;
		value_._42 += ty;
		value_._43 += tz;

		return *this;
	}

	Direct3D11Matrix& set_rotation_x( UnitType r )
	{
		value_ = XMMatrixRotationX( r );

		return *this;
	}

	Direct3D11Matrix& set_rotation_y( UnitType r )
	{
		value_ = XMMatrixRotationY( r );

		return *this;
	}

	Direct3D11Matrix& set_rotation_z( UnitType r )
	{
		value_ = XMMatrixRotationZ( r );

		return *this;
	}

	Direct3D11Matrix& set_rotation_xyz( UnitType rx, UnitType ry, UnitType rz )
	{
		value_ = XMMatrixRotationX( rx ) * XMMatrixRotationY( ry ) * XMMatrixRotationZ( rz );

		return *this;
	}

	Direct3D11Matrix& set_rotation_roll_pitch_yaw( UnitType rx, UnitType ry, UnitType rz )
	{
		value_ = XMMatrixRotationRollPitchYaw( rx, ry, rz );

		return *this;
	}

	Direct3D11Matrix& set_rotation_quaternion( const VectorType& q )
	{
		value_ = XMMatrixRotationQuaternion( q );

		return *this;
	}

	Direct3D11Matrix& set_scaling( UnitType sx, UnitType sy, UnitType sz )
	{
		value_ = XMMatrixScaling( sx, sy, sz );

		return *this;
	}

	Direct3D11Matrix& set_look_at( const VectorType& eye, const VectorType& at, const VectorType& up )
	{
		value_ = XMMatrixLookAtLH( eye, at, up );

		return *this;
	}

	Direct3D11Matrix inverse() const
	{
		XMVECTOR v;
		return Direct3D11Matrix( XMMatrixInverse( & v, value_ ) );
	}

	Direct3D11Matrix transpose() const
	{
		return Direct3D11Matrix( XMMatrixTranspose( value_ ) );
	}

	Direct3D11Matrix& set_perspective_fov( UnitType fov_angle_y, UnitType aspect_ratio, UnitType near_z, UnitType far_z )
	{
		value_ = XMMatrixPerspectiveFovLH( fov_angle_y, aspect_ratio, near_z, far_z );

		return *this;
	}

	Direct3D11Matrix& set_orthographic( UnitType w, UnitType h, UnitType near_z, UnitType far_z )
	{
		value_ = XMMatrixOrthographicLH( w, h, near_z, far_z );

		return *this;
	}

	Direct3D11Matrix& operator *= ( const Direct3D11Matrix& m )
	{
		value_ *= m.value_;

		return *this;
	}

	Direct3D11Matrix operator * ( const Direct3D11Matrix& m ) const
	{
		return Direct3D11Matrix( value_ * m.value_ );
	}

	friend Direct3D11Vector operator * ( const Direct3D11Vector& v, const Direct3D11Matrix& m )
	{
		return XMVector4Transform( v.value_, m.value_ );
	}

	operator XMMATRIX () const { return value_; }


	static Direct3D11Matrix identity()
	{
		Direct3D11Matrix m;
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

}; // class Direct3D11Matrix

#endif // DIRECT_3D_11_MATRIX_H
