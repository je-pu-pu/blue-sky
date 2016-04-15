#ifndef DIRECT_3D_11_MATRIX_H
#define DIRECT_3D_11_MATRIX_H

#include "Direct3D11Common.h"
#include "Direct3D11Vector.h"

/**
 * Direct3D 11 Matrix
 *
 */
class Direct3D11Matrix
{
public:
	typedef float			UnitType;
	typedef int				IntType;
	typedef unsigned int	UintType;

	typedef Direct3D11Vector	VectorType;

private:
	alignas( 16 ) DirectX::XMMATRIX value_;

public:
	Direct3D11Matrix()
	{
		
	}

	Direct3D11Matrix( const DirectX::XMMATRIX& m )
	{
		value_ = m;
	}

	Direct3D11Matrix(DirectX::XMFLOAT4X4 m )
	{
		value_ = DirectX::XMLoadFloat4x4( & m );
	}

	Direct3D11Matrix& operator = ( const Direct3D11Matrix& m )
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

	Direct3D11Matrix& set(
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

	Direct3D11Matrix& set_identity()
	{
		value_ = DirectX::XMMatrixIdentity();

		return *this;
	}

	Direct3D11Matrix& set_translation( UnitType tx, UnitType ty, UnitType tz )
	{
		value_ = DirectX::XMMatrixTranslation( tx, ty, tz );

		return *this;
	}

	Direct3D11Matrix& add_translation( UnitType tx, UnitType ty, UnitType tz )
	{
		value_ *= DirectX::XMMatrixTranslation( tx, ty, tz );

		return *this;
	}

	Direct3D11Matrix& set_rotation_x( UnitType r )
	{
		value_ = DirectX::XMMatrixRotationX( r );

		return *this;
	}

	Direct3D11Matrix& set_rotation_y( UnitType r )
	{
		value_ = DirectX::XMMatrixRotationY( r );

		return *this;
	}

	Direct3D11Matrix& set_rotation_z( UnitType r )
	{
		value_ = DirectX::XMMatrixRotationZ( r );

		return *this;
	}

	Direct3D11Matrix& set_rotation_xyz( UnitType rx, UnitType ry, UnitType rz )
	{
		value_ = DirectX::XMMatrixRotationX( rx ) * DirectX::XMMatrixRotationY( ry ) * DirectX::XMMatrixRotationZ( rz );

		return *this;
	}

	Direct3D11Matrix& set_rotation_roll_pitch_yaw( UnitType rx, UnitType ry, UnitType rz )
	{
		value_ = DirectX::XMMatrixRotationRollPitchYaw( rx, ry, rz );

		return *this;
	}

	Direct3D11Matrix& set_rotation_quaternion( const VectorType& q )
	{
		value_ = DirectX::XMMatrixRotationQuaternion( q );

		return *this;
	}

	Direct3D11Matrix& set_scaling( UnitType sx, UnitType sy, UnitType sz )
	{
		value_ = DirectX::XMMatrixScaling( sx, sy, sz );

		return *this;
	}

	Direct3D11Matrix& set_look_at( const VectorType& eye, const VectorType& at, const VectorType& up )
	{
		value_ = DirectX::XMMatrixLookAtLH( eye, at, up );

		return *this;
	}

	Direct3D11Matrix inverse() const
	{
		DirectX::XMVECTOR v;
		return Direct3D11Matrix( XMMatrixInverse( & v, value_ ) );
	}

	Direct3D11Matrix transpose() const
	{
		return Direct3D11Matrix( XMMatrixTranspose( value_ ) );
	}

	Direct3D11Matrix& set_perspective_fov( UnitType fov_angle_y, UnitType aspect_ratio, UnitType near_z, UnitType far_z )
	{
		value_ = DirectX::XMMatrixPerspectiveFovLH( fov_angle_y, aspect_ratio, near_z, far_z );

		return *this;
	}

	Direct3D11Matrix& set_orthographic( UnitType w, UnitType h, UnitType near_z, UnitType far_z )
	{
		value_ = DirectX::XMMatrixOrthographicLH( w, h, near_z, far_z );

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

	friend Direct3D11Vector& operator *= ( Direct3D11Vector& v, const Direct3D11Matrix& m )
	{
		v = v * m;

		return v;
	}

	operator DirectX::XMMATRIX () const { return value_; }


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
