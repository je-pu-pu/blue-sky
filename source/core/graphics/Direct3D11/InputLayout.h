#pragma once

#include <core/graphics/InputLayout.h>
#include <core/DirectX.h>
#include <d3d11.h>

namespace core::graphics::direct_3d_11
{

/**
 * Direct3D 11 Effect のラッパークラス
 *
 */
class InputLayout : public core::graphics::InputLayout
{
private:
	com_ptr< ID3D11InputLayout > input_layout_;

public:
	InputLayout( ID3D11InputLayout* input_layout )
		: input_layout_( input_layout )
	{
		
	}

	ID3D11InputLayout* get_input_layout() const
	{
		return input_layout_.get();
	}

}; // class InputLayout

} // namespace namespace core::graphics::direct_3d_11
