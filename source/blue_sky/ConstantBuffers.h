#pragma once

#include <blue_sky/type.h>
#include <core/ConstantBuffers.h>

namespace blue_sky
{

struct GameConstantBufferData
{
	float_t screen_width;
	float_t screen_height;
	float_t dummy[ 2 ];
};

struct FrameConstantBufferData
{
	Matrix view;
	Matrix projection;
	Vector light;

	float_t time = 0.f;
	u32_t time_beat = 0;
	float_t tess_factor = 1.f;
	float_t beat_progress = 0.f; // ビート進行度 (1.0 → 0.0)
};

struct FrameDrawingConstantBufferData
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

using ObjectConstantBufferData		= core::ObjectConstantBufferData;
using BoneConstantBufferData		= core::BoneConstantBufferData;

using GameConstantBuffer			= core::graphics::direct_3d_11::ConstantBufferTyped< GameConstantBufferData, 0 >;
using FrameConstantBuffer			= core::graphics::direct_3d_11::ConstantBufferWithData< FrameConstantBufferData, 1 >;
using FrameDrawingConstantBuffer	= core::graphics::direct_3d_11::ConstantBufferWithData< FrameDrawingConstantBufferData, 4 >;
using ObjectConstantBuffer			= core::ObjectConstantBuffer;
using ObjectConstantBufferWithData	= core::ObjectConstantBufferWithData;
using BoneConstantBuffer			= core::BoneConstantBuffer;

} // namespace blue_sky
