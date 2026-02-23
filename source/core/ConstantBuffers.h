#pragma once

#include <core/type.h>
#include <core/graphics/Direct3D11/ConstantBufferWithData.h>

namespace core
{

struct ObjectConstantBufferData
{
	Matrix world;
	Color color;

	ObjectConstantBufferData()
		: color( 0.f, 0.f, 0.f, 0.f )
	{ }
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
