/**
 * 色収差ポストエフェクト
 */

float4 ps_post_effect_chromatic_aberrration( COMMON_POS_UV input ) : SV_Target
{
	float2 h = ( 0.5f, 0.5f );

	return float4(
		source_texture.Sample( texture_sampler, ( input.TexCoord - h ) * 1.00f + h ).r,
		source_texture.Sample( texture_sampler, ( input.TexCoord - h ) * 0.99f + h ).g,
		source_texture.Sample( texture_sampler, ( input.TexCoord - h ) * 0.98f + h ).b,
		1.f
	);
}

technique11 post_effect_chromatic_aberrration
{
	pass main
	{
		SetVertexShader( CompileShader( vs_4_0, vs_post_effect() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_post_effect_chromatic_aberrration() ) );
	}
}
