cbuffer ObjectConstantBuffer : register( b0 )
{
	float UvFactor;			// UV ŒW”	( default : 20.f )
	float TimeFactor;		// ŠÔŒW”	( default : 10.f )
	float Gain1;			// U• 1	( default : 0.001f )
	float Gain2;			// U• 2	( default : 0.0005f )
	float Gain3;			// U• 3	( default : 0.00025f )
};

/**
 * è•`‚«•—•ÏŠ·ƒ|ƒXƒgƒGƒtƒFƒNƒg
 */

float4 ps_post_effect_hand_drawing( COMMON_POS_UV input ) : SV_Target
{
	const float2 uv = input.TexCoord * UvFactor + Time * TimeFactor;

	float2 offset = float2(
		sin( uv.y ) * Gain1 + sin( uv.y * 2.3 ) * Gain2 + sin( uv.y * 5.9 ) * Gain3,
		sin( uv.x ) * Gain1 + sin( uv.x * 2.5 ) * Gain2 + sin( uv.x * 6.2 ) * Gain3
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
