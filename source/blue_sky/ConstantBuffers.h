#pragma once

#include <blue_sky/type.h>
#include <core/graphics/Direct3D11/ConstantBuffer.h>

namespace blue_sky
{

template< uint_t N >
struct BaseConstantBufferData
{
	static const uint_t SLOT = N;
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

	float_t time = 0.f;
	u32_t time_beat = 0;
	float_t tess_factor = 1.f;
	float_t dummy = 0.f;
};

struct ObjectConstantBufferData : public BaseConstantBufferData< 2 >
{
	Matrix world;
	Color color;

	ObjectConstantBufferData()
		: color( 0.f, 0.f, 0.f, 0.f )
	{ }
};

struct FrameDrawingConstantBufferData : public BaseConstantBufferData< 4 >
{
	Color shadow_color;
	Color shadow_paper_color;

	float_t accent;
	u32_t line_type;
	float_t dummy[ 2 ] = { 0, 0 };

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

using GameConstantBuffer			= core::graphics::direct_3d_11::ConstantBuffer< GameConstantBufferData >;
using FrameConstantBuffer			= core::graphics::direct_3d_11::ConstantBufferWithData< FrameConstantBufferData >;
using FrameDrawingConstantBuffer	= core::graphics::direct_3d_11::ConstantBufferWithData< FrameDrawingConstantBufferData >;
using ObjectConstantBuffer			= core::graphics::direct_3d_11::ConstantBuffer< ObjectConstantBufferData >;
using ObjectConstantBufferWithData	= core::graphics::direct_3d_11::ConstantBufferWithData< ObjectConstantBufferData >;
using BoneConstantBuffer			= core::graphics::direct_3d_11::ConstantBuffer< BoneConstantBufferData >;

} // namespace blue_sky
