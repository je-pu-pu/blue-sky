#pragma once

#include <core/graphics/PrimitiveTopology.h>

#include <d3d11.h>

namespace core::graphics::direct_3d_11
{

using PrimitiveTopology = core::graphics::PrimitiveTopology;

/// エンジンのプリミティブの種類 → D3D11 の値
inline D3D11_PRIMITIVE_TOPOLOGY to_d3d_primitive_topology( PrimitiveTopology topology )
{
	switch ( topology )
	{
		case PrimitiveTopology::POINT_LIST:					return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
		case PrimitiveTopology::LINE_LIST:					return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
		case PrimitiveTopology::TRIANGLE_LIST:				return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case PrimitiveTopology::PATCH_LIST_3_CONTROL_POINT:	return D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
	}

	return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
}

} // namespace core::graphics::direct_3d_11
