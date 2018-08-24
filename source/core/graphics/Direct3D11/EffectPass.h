#pragma once

#include <core/graphics/EffectPass.h>
#include <d3d11.h>

class Direct3D11;

struct ID3DX11EffectPass;

namespace core::graphics::direct_3d_11
{

/**
 * Direct3D 11 Effect Pass のラッパークラス
 *
 */
class EffectPass : public core::graphics::EffectPass
{
public:
	using Direct3D		= Direct3D11;
	using InputLayout	= ID3D11InputLayout;

private:
	Direct3D*						direct_3d_;			
	ID3DX11EffectPass*				pass_;

public:
	EffectPass( Direct3D*, ID3DX11EffectPass* );
	~EffectPass();
	
	InputLayout* create_input_layout( D3D11_INPUT_ELEMENT_DESC[], UINT );

	void apply() const override;

}; // class EffectPass

} // namespace namespace core::graphics::direct_3d_11
