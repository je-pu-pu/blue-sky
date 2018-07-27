#pragma once

#include <blue_sky/graphics/Direct3D11/ShaderResource.h>
#include <core/type.h>

/// @todo ConstantBuffer -> ShaderResource Ç…ñºëOïœçXÇ∑ÇÈ

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

	float_t time;
	u32_t time_beat;
	float_t tess_factor = 1.f;
	float_t dummy;
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

typedef graphics::direct_3d_11::ShaderResource< GameConstantBufferData > GameConstantBuffer;
typedef graphics::direct_3d_11::ShaderResourceWithData< FrameConstantBufferData > FrameConstantBuffer;
typedef graphics::direct_3d_11::ShaderResourceWithData< FrameDrawingConstantBufferData > FrameDrawingConstantBuffer;
typedef graphics::direct_3d_11::ShaderResource< ObjectConstantBufferData > ObjectConstantBuffer;
typedef graphics::direct_3d_11::ShaderResourceWithData< ObjectConstantBufferData > ObjectConstantBufferWithData;
typedef graphics::direct_3d_11::ShaderResource< BoneConstantBufferData > BoneConstantBuffer;

} // namespace blue_sky
