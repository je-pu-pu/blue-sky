/**
 * 手描き風変換ポストエフェクト
 */

float4 ps_post_effect_hand_drawing( COMMON_POS_UV input ) : SV_Target
{
	const float2 uv = input.TexCoord * 20.f + Time * 10.f;
	const float gain1 = 0.001f;
	const float gain2 = 0.0005f;
	const float gain3 = 0.00025f;
	// const float gain4 = 

	float2 offset = float2(
		sin( uv.y ) * gain1 + sin( uv.y * 2.3 ) * gain2 + sin( uv.y * 5.9 ) * gain3,
		sin( uv.x ) * gain1 + sin( uv.x * 2.5 ) * gain2 + sin( uv.x * 6.2 ) * gain3
	);

	return source_texture.Sample( texture_sampler, input.TexCoord + offset );
}

technique11 post_effect_hand_drawing
{
	pass main
	{
		SetVertexShader( CompileShader( vs_4_0, vs_post_effect() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_post_effect_hand_drawing() ) );
	}
}
