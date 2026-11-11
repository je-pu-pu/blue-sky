#pragma once

#include <core/graphics/PixelFormat.h>

#include <d3d11.h>

namespace core::graphics::direct_3d_11
{
	using PixelFormat = core::graphics::PixelFormat;

	/// エンジンのピクセルフォーマット → DXGI_FORMAT
	inline DXGI_FORMAT to_dxgi_format( PixelFormat format )
	{
		switch ( format )
		{
			case PixelFormat::R8G8B8A8_UNORM:	return DXGI_FORMAT_R8G8B8A8_UNORM;
			case PixelFormat::R8_UINT:			return DXGI_FORMAT_R8_UINT;
			case PixelFormat::R32G32_FLOAT:		return DXGI_FORMAT_R32G32_FLOAT;
		}

		return DXGI_FORMAT_UNKNOWN;
	}
}
