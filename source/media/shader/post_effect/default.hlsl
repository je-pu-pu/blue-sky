COMMON_POS_UV vs_post_effect_default( COMMON_POS_NORM_UV input )
{
	COMMON_POS_UV output;

	output.Position = input.Position;
	output.TexCoord = input.TexCoord;

	return output;
}

float4 ps_post_effect_default( COMMON_POS_UV input ) : SV_Target
{
	return float4(1.f, 0.f, 0.f, 1.f);
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