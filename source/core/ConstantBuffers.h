#pragma once

#include <core/type.h>
#include <core/graphics/Direct3D11/ConstantBufferWithData.h>

namespace core
{

struct ObjectConstantBufferData
{
	Matrix world;
	Color color;
	Matrix prev_world;	///< 前フレームの world ( モーションベクトル算出用 / 末尾追記で既存オフセット不変 )

	ObjectConstantBufferData()
		: color( 0.f, 0.f, 0.f, 0.f )
	{
		prev_world.set_identity();
	}
};

struct BoneConstantBufferData
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

using ObjectConstantBuffer			= core::graphics::direct_3d_11::ConstantBufferTyped< ObjectConstantBufferData, 2 >;
using ObjectConstantBufferWithData	= core::graphics::direct_3d_11::ConstantBufferWithData< ObjectConstantBufferData, 2 >;
using BoneConstantBuffer			= core::graphics::direct_3d_11::ConstantBufferTyped< BoneConstantBufferData, 5 >;

} // namespace core
