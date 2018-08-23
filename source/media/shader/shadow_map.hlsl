/**
 * シャドウマップへのレンダリング
 *
 */
float4 vs_shadow_map( COMMON_POS_NORM_UV input ) : SV_POSITION
{
	float4 output;

	output = mul( input.Position, World );
    output = mul( output, ShadowViewProjection[ 0 ] );

	return output;
}

/**
 * シャドウマップへのスキンメッシュのレンダリング
 *
 */
float4 vs_shadow_map_skin( VS_SKIN_INPUT input ) : SV_POSITION
{
	float4 output = common_skinning_pos( input.Position, input.Bone, input.Weight );

	output = mul( output, World );
    output = mul( output, ShadowViewProjection[ 0 ] );

	return output;
}

technique11 shadow_map
{
	pass main
	{
		SetBlendState( NoBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_shadow_map() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( NULL );

		RASTERIZERSTATE = Shadow;
	}
}

technique11 shadow_map_skin
{
	pass main
	{
		SetBlendState( NoBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_shadow_map_skin() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( NULL );

		RASTERIZERSTATE = Shadow;
	}
}