float4 ps_transparent( COMMON_POS input ) : SV_Target
{
	return float4( 1.0f, 0.0f, 0.0f, 0.5f );
	// return ObjectColor;
}

technique11 transparent
{
	pass main
    {
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

        SetVertexShader( CompileShader( vs_4_0, vs_common_wvp_pos_norm_uv_to_pos() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_transparent() ) );

		RASTERIZERSTATE = Default;
    }
}
