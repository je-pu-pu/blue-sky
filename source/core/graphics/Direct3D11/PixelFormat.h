#pragma once

#include <d3d11.h>

namespace core::graphics::direct_3d_11
{
	/**
	 * ピクセルフォーマット
	 *
	 */
	enum class PixelFormat
	{
		R8G8B8A8_UNORM = DXGI_FORMAT_R8G8B8A8_UNORM,
		R8G8B8A8_SNORM = DXGI_FORMAT_R8G8B8A8_SNORM,
		B8G8R8A8_UNORM = DXGI_FORMAT_B8G8R8A8_UNORM,
	};
}