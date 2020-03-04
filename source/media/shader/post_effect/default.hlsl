/**
 * 何もしないポストエフェクト
 */

float4 ps_post_effect_default( COMMON_POS_UV input ) : SV_Target
{
	return source_texture.Sample( texture_sampler, input.TexCoord );
}

technique11 post_effect_default
{
	pass main
	{
		SetVertexShader( CompileShader( vs_4_0, vs_post_effect() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_post_effect_default() ) );
	}
}