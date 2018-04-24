DepthStencilState SkyBoxDepthStencilState
{
	DepthWriteMask = ALL;
	DepthFunc = LESS_EQUAL;
};

struct PS_SKY_BOX_OUTPUT
{
	float4 Color : SV_Target;
	float Depth  : SV_Depth;
};

PS_SKY_BOX_OUTPUT ps_sky_box( COMMON_POS_UV input )
{
	PS_SKY_BOX_OUTPUT output;

	output.Color = model_texture.Sample( texture_sampler, input.TexCoord ); // * ObjectColor;
	output.Depth = 1;

	return output;
}

// ----------------------------------------
// for SkyBox
// ----------------------------------------
technique11 sky_box
{
	pass main
	{
		SetDepthStencilState( SkyBoxDepthStencilState, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_common_wvp_pos_norm_uv_to_pos_uv() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_sky_box() ) );

		RASTERIZERSTATE = Default;
	}
}