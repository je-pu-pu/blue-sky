/*
RasterizerState Main2dRasterizerState
{
	CullMode = None;
};

/// @todo 2D の描画に法線を使っているのは無駄なのでなんとかする
struct VS_2D_INPUT
{
	float4 Position : SV_POSITION;
	float3 Normal   : NORMAL0;
	float2 TexCoord : TEXCOORD0;
};
*/

struct PS_2D_INPUT
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
};


// ----------------------------------------
// for 2D ( Fader, debug ウィンドウで使用 )
// ----------------------------------------
PS_2D_INPUT vs_2d( VS_INPUT input )
{
	PS_2D_INPUT output;

	output.Position = input.Position;
	output.TexCoord = input.TexCoord;

	return output;
}

float4 ps_2d( PS_2D_INPUT input ) : SV_Target
{
	return model_texture.Sample( texture_sampler, input.TexCoord ) + ObjectColor;
}

technique11 main2d
{
	pass main
	{
		SetVertexShader( CompileShader( vs_4_0, vs_2d() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_2d() ) );

		// RASTERIZERSTATE = Main2dRasterizerState;
	}
}

float4 ps_debug_shadow_map_texture( PS_2D_INPUT input ) : SV_Target
{
	const float value = model_texture.Sample( texture_sampler, input.TexCoord ).x;

	return float4( value, value, value, 1.f );
}

technique11 debug_shadow_map_texture
{
	pass main
	{
		SetVertexShader( CompileShader( vs_4_0, vs_2d() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_debug_shadow_map_texture() ) );
	}
}