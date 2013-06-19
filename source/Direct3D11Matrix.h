#ifndef DIRECT_3D_11_MATRIX_H
#define DIRECT_3D_11_MATRIX_H

#include <d3d11.h>
#include <xnamath.h>

class Direct3D11Vector;

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

private:
	// __declspec( align( 16) ) XMMATRIX value_;
	XMFLOAT4X4				value_;

public:
	Direct3D11Matrix()
	{
		set_identity();
	}

	Direct3D11Matrix( XMFLOAT4X4 m )
	{
		value_ = m;
	}

	/*
	Direct3D11Matrix( XMMATRIX m )
	{
		XMStoreFloat4x4( & value_, m );
	}
	*/

	Direct3D11Matrix& operator = ( const Direct3D11Matrix& m )
	{
		value_ = m.value_;

		return *this;
	}
	
	UnitType get( int y, int x )
	{
		return value_.m[ y ][ x ];
	}

	void set(
		UnitType m00, UnitType m01, UnitType m02, UnitType m03,
		UnitType m10, UnitType m11, UnitType m12, UnitType m13,
		UnitType m20, UnitType m21, UnitType m22, UnitType m23,
		UnitType m30, UnitType m31, UnitType m32, UnitType m33 )
	{
		XMStoreFloat4x4( & value_, XMMatrixSet( m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33 ) );
	}

	void set_identity()
	{
		// value_ = XMMatrixIdentity();

		XMStoreFloat4x4( & value_, XMMatrixIdentity() );
	}

	void set_translation( UnitType tx, UnitType ty, UnitType tz )
	{
		// value_ = XMMatrixTranslation( tx, ty, tz );

		XMStoreFloat4x4( & value_, XMMatrixTranslation( tx, ty, tz ) );
	}

	void set_rotation_x( UnitType r )
	{
		XMStoreFloat4x4( & value_, XMMatrixRotationX( r ) );
	}

	void set_rotation_y( UnitType r )
	{
		XMStoreFloat4x4( & value_, XMMatrixRotationY( r ) );
	}

	void set_rotation_z( UnitType r )
	{
		XMStoreFloat4x4( & value_, XMMatrixRotationZ( r ) );
	}

	void set_rotation_roll_pitch_yaw( UnitType rx, UnitType ry, UnitType rz )
	{
		XMStoreFloat4x4( & value_, XMMatrixRotationRollPitchYaw( rx, ry, rz ) );
	}

	void set_scaling( UnitType sx, UnitType sy, UnitType sz )
	{
		// value_ = XMMatrixScaling( sx, sy, sz );

		XMStoreFloat4x4( & value_, XMMatrixScaling( sx, sy, sz ) );
	}

	Direct3D11Matrix inverse() const
	{
		Direct3D11Matrix m;

		XMVECTOR v;
		XMStoreFloat4x4( & m.value_, XMMatrixInverse( & v, XMLoadFloat4x4( & value_ ) ) );

		return m;
	}

	Direct3D11Matrix transpose() const
	{
		Direct3D11Matrix m;
		XMStoreFloat4x4( & m.value_, XMMatrixTranspose( XMLoadFloat4x4( & value_ ) ) );

		return m;
	}


	Direct3D11Matrix operator * ( const Direct3D11Matrix& m ) const
	{
		// return Direct3D11Matrix( value_ * m.value_ );

		Direct3D11Matrix d11m;
		XMStoreFloat4x4( & d11m.value_, XMLoadFloat4x4( & value_ ) * XMLoadFloat4x4( & m.value_ ) );
		
		return d11m;
	}

	friend Direct3D11Vector operator * ( const Direct3D11Vector&, const Direct3D11Matrix& );

	operator XMMATRIX () const { return XMLoadFloat4x4( & value_ ); }

	XMFLOAT4X4& get() { return value_; }
	const XMFLOAT4X4& get() const { return value_; }

}; // class Direct3D11Matrix

#endif // DIRECT_3D_11_MATRIX_H
