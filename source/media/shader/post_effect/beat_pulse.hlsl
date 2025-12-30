/**
 * ビート同期ポストエフェクト
 *
 * BeatProgress (1.0 -> 0.0) を使用して以下の演出を行う:
 * - 色収差 (ビート時に強く)
 * - ビネット (ビート時に弱く)
 * - 明るさブースト (ビート時に明るく)
 */

float4 ps_post_effect_beat_pulse( COMMON_POS_UV input ) : SV_Target
{
	float2 uv = input.TexCoord;
	float2 center = float2( 0.5f, 0.5f );

	// BeatProgress: 1.0 (ビート直後) -> 0.0 (次のビート直前)
	float beat = BeatProgress;

	// イージング (急激に減衰)
	float beatPow = pow( beat, 0.5f );

	// --- 色収差 ---
	// ビート時に強い色収差、減衰していく
	float aberrationStrength = beatPow * 0.02f;

	float2 dir = uv - center;
	float3 color;
	color.r = source_texture.Sample( texture_sampler, uv + dir * aberrationStrength ).r;
	color.g = source_texture.Sample( texture_sampler, uv ).g;
	color.b = source_texture.Sample( texture_sampler, uv - dir * aberrationStrength ).b;

	// --- ビネット ---
	// ビート時にビネットが弱く (視界が開く)、減衰すると強く
	float dist = length( dir ) * 1.4f; // 0.0 (中央) ~ 1.0 (端)
	float vignetteBase = 0.3f;
	float vignetteBeat = 0.2f * beatPow; // ビート時にビネットを減らす
	float vignette = 1.0f - smoothstep( 0.5f, 1.0f, dist ) * ( vignetteBase - vignetteBeat );

	color *= vignette;

	// --- 明るさブースト ---
	// ビート時に少し明るく
	float brightness = 1.0f + beatPow * 0.15f;
	color *= brightness;

	return float4( color, 1.0f );
}

technique11 post_effect_beat_pulse
{
	pass main
	{
		SetDepthStencilState( NoDepthTest, 0xFFFFFFFF );
		SetVertexShader( CompileShader( vs_4_0, vs_post_effect() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_post_effect_beat_pulse() ) );

		RASTERIZERSTATE = Default;
	}
}
