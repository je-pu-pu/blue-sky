#include "EffectPass.h"
#include "Direct3D11.h"

namespace core::graphics::direct_3d_11
{

EffectPass::EffectPass( Direct3D* direct_3d, EffectPassSetup setup )
	: direct_3d_( direct_3d )
	, setup_( std::move( setup ) )
{

}

EffectPass::~EffectPass()
{

}

EffectPass::InputLayout* EffectPass::create_input_layout( D3D11_INPUT_ELEMENT_DESC layout[], UINT layout_array_size )
{
	const auto& vs = setup_.shader_list[ static_cast< size_t >( effect::ShaderStage::VS ) ];

	if ( ! vs || ! *vs || ! ( *vs )->byte_code )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( "入力レイアウトを作るパスに頂点シェーダーがありません" );
	}

	ID3DBlob* byte_code = ( *vs )->byte_code.Get();
	InputLayout* input_layout = nullptr;

	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateInputLayout( layout, layout_array_size, byte_code->GetBufferPointer(), byte_code->GetBufferSize(), & input_layout ) );

	return input_layout;
}

void EffectPass::apply() const
{
	auto* context = direct_3d_->getImmediateContext();

	if ( setup_.has_blend_state )
	{
		context->OMSetBlendState( setup_.blend_state.Get(), setup_.blend_factor.data(), setup_.sample_mask );
	}

	if ( setup_.has_depth_stencil_state )
	{
		context->OMSetDepthStencilState( setup_.depth_stencil_state.Get(), setup_.stencil_ref );
	}

	if ( setup_.has_rasterizer_state )
	{
		context->RSSetState( setup_.rasterizer_state.Get() );
	}

	for ( size_t n = 0; n < setup_.shader_list.size(); n++ )
	{
		const auto& entry = setup_.shader_list[ n ];

		if ( ! entry )
		{
			continue;	// 指定なし（触らない）
		}

		const EffectShader* shader = entry->get();
		ID3D11DeviceChild* object = shader ? shader->object.Get() : nullptr;

		switch ( static_cast< effect::ShaderStage >( n ) )
		{
			case effect::ShaderStage::VS: context->VSSetShader( static_cast< ID3D11VertexShader* >( object ), nullptr, 0 ); break;
			case effect::ShaderStage::HS: context->HSSetShader( static_cast< ID3D11HullShader* >( object ), nullptr, 0 ); break;
			case effect::ShaderStage::DS: context->DSSetShader( static_cast< ID3D11DomainShader* >( object ), nullptr, 0 ); break;
			case effect::ShaderStage::GS: context->GSSetShader( static_cast< ID3D11GeometryShader* >( object ), nullptr, 0 ); break;
			case effect::ShaderStage::PS: context->PSSetShader( static_cast< ID3D11PixelShader* >( object ), nullptr, 0 ); break;
		}

		if ( ! shader )
		{
			continue;
		}

		// サンプラーは effect が結び付けていた（hlsl に register を書いていない）ので、ここでスロットに結び付ける
		for ( const auto& [ slot, sampler ] : shader->sampler_list )
		{
			ID3D11SamplerState* s = sampler.Get();

			switch ( shader->stage )
			{
				case effect::ShaderStage::VS: context->VSSetSamplers( slot, 1, & s ); break;
				case effect::ShaderStage::HS: context->HSSetSamplers( slot, 1, & s ); break;
				case effect::ShaderStage::DS: context->DSSetSamplers( slot, 1, & s ); break;
				case effect::ShaderStage::GS: context->GSSetSamplers( slot, 1, & s ); break;
				case effect::ShaderStage::PS: context->PSSetSamplers( slot, 1, & s ); break;
			}
		}
	}
}

} // namespace core::graphics::direct_3d_11
