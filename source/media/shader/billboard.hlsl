float4 ps_billboard( COMMON_POS_UV input ) : SV_Target
{
	float4 output = model_texture.Sample( texture_sampler, input.TexCoord ) * ObjectColor;

	clip( output.a - 0.0001f );

	return output;
}

// ----------------------------------------
// for Billboard
// ----------------------------------------
technique11 billboard
{
	pass main
	{
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_common_wvp_pos_norm_uv_to_pos_uv() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_billboard() ) );
		// SetPixelShader( CompileShader( ps_4_0, ps_flat_with_flicker() ) );

		RASTERIZERSTATE = Default;
	}
}