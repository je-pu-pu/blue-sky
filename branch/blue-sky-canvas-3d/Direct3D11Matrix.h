#ifndef DIRECT_3D_11_MATRIX_H
#define DIRECT_3D_11_MATRIX_H

#include <d3d11.h>
#include <xnamath.h>

/**
 * Direct3D 11 Vector
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

	void set_identity()
	{
		// value_ = XMMatrixIdentity();

		XMStoreFloat4x4( & value_, XMMatrixIdentity() );
	}

	void set_scaling( UnitType sx, UnitType sy, UnitType sz )
	{
		// value_ = XMMatrixScaling( sx, sy, sz );

		XMStoreFloat4x4( & value_, XMMatrixScaling( sx, sy, sz ) );
	}

	void set_translation( UnitType tx, UnitType ty, UnitType tz )
	{
		// value_ = XMMatrixTranslation( tx, ty, tz );

		XMStoreFloat4x4( & value_, XMMatrixTranslation( tx, ty, tz ) );
	}

	Direct3D11Matrix operator * ( const Direct3D11Matrix& m )
	{
		// return Direct3D11Matrix( value_ * m.value_ );

		Direct3D11Matrix d11m;
		XMStoreFloat4x4( & d11m.value_, XMLoadFloat4x4( & value_ ) * XMLoadFloat4x4( & m.value_ ) );
		
		return d11m;
	}

	XMFLOAT4X4& get() { return value_; }
	const XMFLOAT4X4& get() const { return value_; }

}; // class Direct3D11Matrix

#endif // DIRECT_3D_11_MATRIX_H
