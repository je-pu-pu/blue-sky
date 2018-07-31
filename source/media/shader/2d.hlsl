/*
RasterizerState Main2dRasterizerState
{
	CullMode = None;
};
*/

/// @todo 2D の描画に法線を使っているのは無駄なのでなんとかする

// ----------------------------------------
// for 2D ( debug ウィンドウで使用 )
// ----------------------------------------
COMMON_POS_UV vs_2d( COMMON_POS_NORM_UV input )
{
	COMMON_POS_UV output;

	output.Position = input.Position;
	output.TexCoord = input.TexCoord;

	return output;
}

float4 ps_2d( COMMON_POS_UV input ) : SV_Target
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

float4 ps_debug_shadow_map_texture( COMMON_POS_UV input ) : SV_Target
{
	// return model_texture.Sample( texture_sampler, input.TexCoord );

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