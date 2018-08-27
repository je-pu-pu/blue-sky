#pragma once

#include <blue_sky/type.h>
#include <core/graphics/Direct3D11/ShaderResource.h>

namespace blue_sky
{

template< uint_t N >
struct BaseShaderResourceData
{
	static const uint_t SLOT = N;
};

struct GameShaderResourceData : public BaseShaderResourceData< 0 >
{
	float_t screen_width;
	float_t screen_height;
	float_t dummy[ 2 ];
};

struct FrameShaderResourceData : public BaseShaderResourceData< 1 >
{
	Matrix view;
	Matrix projection;
	Vector light;

	float_t time;
	u32_t time_beat;
	float_t tess_factor = 1.f;
	float_t dummy;
};

struct ObjectShaderResourceData : public BaseShaderResourceData< 2 >
{
	Matrix world;
	Color color;

	ObjectShaderResourceData()
		: color( 0.f, 0.f, 0.f, 0.f )
	{ }
};

struct FrameDrawingShaderResourceData : public BaseShaderResourceData< 4 >
{
	Color shadow_color;
	Color shadow_paper_color;

	float_t accent;
	u32_t line_type;
	float_t dummy[ 2 ];

	FrameDrawingShaderResourceData()
		: shadow_color( 0.f, 0.f, 0.f, 0.f )
		, shadow_paper_color( 0.f, 0.f, 0.f, 0.f )
		, accent( 0.f )
		, line_type( 0 )
	{ }
};

struct BoneShaderResourceData : public BaseShaderResourceData< 5 >
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

using GameShaderResource			= core::graphics::direct_3d_11::ShaderResource< GameShaderResourceData >;
using FrameShaderResource			= core::graphics::direct_3d_11::ShaderResourceWithData< FrameShaderResourceData >;
using FrameDrawingShaderResource	= core::graphics::direct_3d_11::ShaderResourceWithData< FrameDrawingShaderResourceData >;
using ObjectShaderResource			= core::graphics::direct_3d_11::ShaderResource< ObjectShaderResourceData >;
using ObjectShaderResourceWithData	= core::graphics::direct_3d_11::ShaderResourceWithData< ObjectShaderResourceData >;
using BoneShaderResource			= core::graphics::direct_3d_11::ShaderResource< BoneShaderResourceData >;

} // namespace blue_sky