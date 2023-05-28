#pragma once

#include <d3d11.h>

namespace core::graphics::direct_3d_11
{

enum class PrimitiveTopology
{
	LINE_LIST = D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
	TRIANGLE_LIST = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
	PATCH_LIST_3_CONTROL_POINT = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST
};

} // namespace core::graphics::direct_3d_11