COMMON_POS_NORM vs_matcap( COMMON_POS_NORM_UV input )
{
	COMMON_POS_NORM output;

	output.Position = common_wvp_pos( input.Position );
	output.Normal = common_w_norm( input.Normal );

	return output;
}

COMMON_POS_NORM vs_matcap_skin( VS_SKIN_INPUT input )
{
	COMMON_POS_NORM output;

	output.Position = common_skinning_pos( input.Position, input.Bone, input.Weight );
	output.Position = common_wvp_pos( output.Position );

	output.Normal = common_skinning_norm( input.Normal, input.Bone, input.Weight );
	output.Normal = common_w_norm( output.Normal );
	
	return output;
}

technique11 matcap
{
	pass main
    {
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

        SetVertexShader( CompileShader( vs_4_0, vs_matcap() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_common_sample_matcap_pos_norm() ) );

		RASTERIZERSTATE = Default;
    }
}

technique11 matcap_skin
{
	pass main
    {
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

        SetVertexShader( CompileShader( vs_4_0, vs_matcap_skin() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, ps_common_sample_matcap_pos_norm() ) );

		RASTERIZERSTATE = Default;
    }
}
