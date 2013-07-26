#ifndef BLUE_SKY_CONSTANT_BUFFER_H
#define BLUE_SKY_CONSTANT_BUFFER_H

#include "Direct3D11ConstantBuffer.h"
#include "Direct3D11Color.h"
#include "Direct3D11Matrix.h"
#include "Direct3D11Vector.h"

namespace blue_sky
{

template< int N >
struct BaseConstantBufferData
{
	typedef Direct3D11Color		Color;
	typedef Direct3D11Matrix	Matrix;
	typedef Direct3D11Vector	Vector;

	static const int DEFAULT_SLOT = N;
};

struct GameConstantBufferData : public BaseConstantBufferData< 0 >
{
	float_t screen_width;
	float_t screen_height;

	float_t dummy[ 2 ];
};

struct FrameConstantBufferData : public BaseConstantBufferData< 1 >
{
	XMMATRIX view;
	XMMATRIX projection;
	Vector light;
	float_t time;
	uint_t time_beat;
};

struct ObjectConstantBufferData : public BaseConstantBufferData< 2 >
{
	XMMATRIX world;
	Color color;

	ObjectConstantBufferData()
		: color( 0.f, 0.f, 0.f, 0.f )
	{ }
};

struct ObjectExtentionConstantBufferData : public BaseConstantBufferData< 3 >
{
	Color color;
};

struct FrameDrawingConstantBufferData : public BaseConstantBufferData< 4 >
{
	float_t accent;
	float_t dummy[ 3 ];
};

struct BoneConstantBufferData : public BaseConstantBufferData< 5 >
{
	static const int MaxBones = 100;

	Matrix bone_matrix[ MaxBones ];

	void set_identity()
	{
		for ( int n = 0; n < MaxBones; ++n )
		{
			bone_matrix[ n ].set_identity();
		}
	}
};

typedef Direct3D11ConstantBuffer< GameConstantBufferData > GameConstantBuffer;
typedef Direct3D11ConstantBuffer< FrameConstantBufferData > FrameConstantBuffer;
typedef Direct3D11ConstantBuffer< FrameDrawingConstantBufferData > FrameDrawingConstantBuffer;
typedef Direct3D11ConstantBuffer< ObjectConstantBufferData > ObjectConstantBuffer;
typedef Direct3D11ConstantBuffer< ObjectExtentionConstantBufferData > ObjectExtentionConstantBuffer;
typedef Direct3D11ConstantBuffer< BoneConstantBufferData > BoneConstantBuffer;

} // namespace blue_sky

#endif // BLUE_SKY_CONSTANT_BUFFER_H