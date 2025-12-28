/**
 * シェーディングありの頂点シェーダー
 */
COMMON_POS_UV_COLOR vs_lit( COMMON_POS_NORM_UV input )
{
	COMMON_POS_UV_COLOR output;

	output.Position = common_wvp_pos( input.Position );
	output.TexCoord = input.TexCoord;
    output.Color = dot( -Light, input.Normal ) * 0.5f + 1.f;

	return output;
}

/**
 * シェーディングありのピクセルシェーダー
 */
float4 ps_lit( COMMON_POS_UV_COLOR input ) : SV_Target
{
	return model_texture.Sample( wrap_texture_sampler, input.TexCoord ) * input.Color;
}

// シェーディングあり・スキニングなし
technique11 lit
{
	pass main
    {
		SetBlendState( NoBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

        SetVertexShader( CompileShader( vs_4_0, vs_lit() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, ps_lit() ) );

		RASTERIZERSTATE = Default;
    }
}
