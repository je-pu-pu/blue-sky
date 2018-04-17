#include "Direct3D11EffectPass.h"
#include "Direct3D11.h"
#include "DirectX.h"

#include "include/d3dx11effect.h"

Direct3D11EffectPass::Direct3D11EffectPass( Direct3D* direct_3d, ID3DX11EffectPass* pass )
	: direct_3d_( direct_3d )
	, pass_( pass )
{

}

Direct3D11EffectPass::~Direct3D11EffectPass()
{

}

Direct3D11EffectPass::InputLayout* Direct3D11EffectPass::create_input_layout( D3D11_INPUT_ELEMENT_DESC layout[], UINT layout_array_size )
{
	D3DX11_PASS_DESC pass_desc;
	pass_->GetDesc( & pass_desc );

	InputLayout* input_layout = 0;

	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateInputLayout( layout, layout_array_size, pass_desc.pIAInputSignature, pass_desc.IAInputSignatureSize, &input_layout ) );

	return input_layout;
}

void Direct3D11EffectPass::apply()
{
	pass_->Apply( 0, direct_3d_->getImmediateContext() );
}
