#ifndef BLUE_SKY_CONSTANT_BUFFER_H
#define BLUE_SKY_CONSTANT_BUFFER_H

#include <core/graphics/Direct3D11/Direct3D11ConstantBuffer.h>
#include <core/graphics/Direct3D11/Direct3D11Color.h>
#include <core/graphics/Direct3D11/Direct3D11Matrix.h>
#include <core/graphics/Direct3D11/Direct3D11Vector.h>

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
	Matrix view;
	Matrix projection;
	Vector light;
	float_t time;
	uint_t time_beat;
};

struct ObjectConstantBufferData : public BaseConstantBufferData< 2 >
{
	Matrix world;
	Color color;

	ObjectConstantBufferData()
		: color( 0.f, 0.f, 0.f, 0.f )
	{ }
};

struct ObjectExtentionConstantBufferData : public BaseConstantBufferData< 3 >
{
	Color color;

	ObjectExtentionConstantBufferData()
		: color( 0.f, 0.f, 0.f, 0.f )
	{ }
};

struct FrameDrawingConstantBufferData : public BaseConstantBufferData< 4 >
{
	Color shadow_color;
	Color shadow_paper_color;
	float_t accent;
	uint_t line_type;
	float_t dummy[ 2 ];

	FrameDrawingConstantBufferData()
		: shadow_color( 0.f, 0.f, 0.f, 0.f )
		, shadow_paper_color( 0.f, 0.f, 0.f, 0.f )
		, accent( 0.f )
		, line_type( 0 )
	{ }
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
typedef Direct3D11ConstantBufferWithData< FrameDrawingConstantBufferData > FrameDrawingConstantBuffer;
typedef Direct3D11ConstantBuffer< ObjectConstantBufferData > ObjectConstantBuffer;
typedef Direct3D11ConstantBufferWithData< ObjectConstantBufferData > ObjectConstantBufferWithData;
typedef Direct3D11ConstantBuffer< ObjectExtentionConstantBufferData > ObjectExtentionConstantBuffer;
typedef Direct3D11ConstantBuffer< BoneConstantBufferData > BoneConstantBuffer;

} // namespace blue_sky

#endif // BLUE_SKY_CONSTANT_BUFFER_H