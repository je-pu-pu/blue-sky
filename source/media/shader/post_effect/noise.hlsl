cbuffer ObjectConstantBuffer : register( b0 )
{
	int Seed;			// ランダムシード
	float Gain;			// 振幅
};

float random( float2 texCoord, int Seed )
{
    return frac(sin(dot(texCoord.xy, float2(12.9898, 78.233)) + Seed) * 43758.5453);
}

/**
 * 変換ポストエフェクト
 */
float4 ps_post_effect_noise( COMMON_POS_UV input ) : SV_Target
{
	float4 c1 = source_texture.Sample( texture_sampler, input.TexCoord + random( input.TexCoord, Seed + 0 ) * Gain );
	float4 c2 = source_texture.Sample( texture_sampler, input.TexCoord + random( input.TexCoord, Seed + 1 ) * Gain );
	float4 c3 = source_texture.Sample( texture_sampler, input.TexCoord + random( input.TexCoord, Seed + 2 ) * Gain );
	float4 c4 = source_texture.Sample( texture_sampler, input.TexCoord + random( input.TexCoord, Seed + 3 ) * Gain );

	return ( c1 + c2 + c3 + c4 ) / 4.f;
}

technique11 post_effect_noise
{
	pass main
	{
		SetDepthStencilState( NoDepthTest, 0xFFFFFFFF );
		SetVertexShader( CompileShader( vs_4_0, vs_post_effect() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_post_effect_noise() ) );

		RASTERIZERSTATE = Default;
	}
}
