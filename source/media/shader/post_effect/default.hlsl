Texture2D source_texture : register( t0 ); /// 前回のレンダリング結果が書き込まれているテクスチャ

COMMON_POS_UV vs_post_effect_default( COMMON_POS_NORM_UV input )
{
	COMMON_POS_UV output;

	output.Position = input.Position;
	output.TexCoord = input.TexCoord;

	return output;
}

float4 ps_post_effect_default( COMMON_POS_UV input ) : SV_Target
{
	// float2 a = source_texture.Sample( texture_sampler, input.TexCoord );
	// return source_texture.Sample( texture_sampler, input.TexCoord + float2( sin( a.x ), sin( a.y ) ) * 0.25f );

	// test
	float len = length( input.TexCoord - 0.5f );
	float a = len * 10.f;

	float2 uv = input.TexCoord + float2( cos( a ), sin( a ) ) * len;
	// return float4( uv, 0, 1 );

	return source_texture.Sample( texture_sampler, uv );

	return source_texture.Sample( texture_sampler, input.TexCoord );
}

technique11 post_effect_default
{
	pass main
	{
		SetVertexShader( CompileShader( vs_4_0, vs_post_effect_default() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_post_effect_default() ) );
	}
}