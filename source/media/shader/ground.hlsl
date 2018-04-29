RasterizerState BackgroundRasterizerState
{
	CullMode = Back;
	DepthBias = 8.f;
	SlopeScaledDepthBias = 8.f;
};

float4 ps_ground( COMMON_POS_UV input ) : SV_Target
{
	return model_texture.Sample( texture_sampler, input.TexCoord ) * ObjectColor;
}

technique11 ground
{
	pass main
	{
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_common_wvp_pos_norm_uv_to_pos_uv() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_ground() ) );

		RASTERIZERSTATE = BackgroundRasterizerState;
	}
}