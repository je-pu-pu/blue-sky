#include "EffectPass.h"
#include "Direct3D11.h"

#include "d3dx11effect.h"

namespace core::graphics::direct_3d_11
{

EffectPass::EffectPass( Direct3D* direct_3d, ID3DX11EffectPass* pass )
	: direct_3d_( direct_3d )
	, pass_( pass )
{

}

EffectPass::~EffectPass()
{

}

EffectPass::InputLayout* EffectPass::create_input_layout( D3D11_INPUT_ELEMENT_DESC layout[], UINT layout_array_size )
{
	D3DX11_PASS_DESC pass_desc;
	pass_->GetDesc( & pass_desc );

	InputLayout* input_layout = 0;

	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateInputLayout( layout, layout_array_size, pass_desc.pIAInputSignature, pass_desc.IAInputSignatureSize, &input_layout ) );

	return input_layout;
}

void EffectPass::apply() const
{
	pass_->Apply( 0, direct_3d_->getImmediateContext() );
}

} // namespace core::graphics::direct_3d_11
