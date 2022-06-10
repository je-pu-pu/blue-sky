Texture2D< uint > source_uint_texture : register( t0 ); /// ポストエフェクト適用前のレンダリング結果が書き込まれているテクスチャ

/**
 * ID -> 色 変換ポストエフェクト
 */
float4 ps_post_effect_id_to_color( COMMON_POS_UV input ) : SV_Target
{
	float4 colors[] = {
		{ 1.f, 0.f, 0.f, 1.f },
		{ 0.f, 1.f, 0.f, 1.f },
		{ 0.f, 0.f, 1.f, 1.f },
		{ 1.f, 1.f, 0.f, 1.f },
		{ 1.f, 0.f, 1.f, 1.f },
		{ 0.f, 1.f, 1.f, 1.f },
	};

	float2 texture_size;

	source_uint_texture.GetDimensions( texture_size.x, texture_size.y );

	uint id = source_uint_texture.Load( float3( input.TexCoord * texture_size, 0 ) );

	return colors[ id % 6 ] * 0.5 + 0.5;
}

technique11 post_effect_id_to_color
{
	pass main
	{
		SetDepthStencilState( NoDepthTest, 0xFFFFFFFF );
		SetVertexShader( CompileShader( vs_4_0, vs_post_effect() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_post_effect_id_to_color() ) );

		RASTERIZERSTATE = Default;
	}
}
