struct PS_FADER_INPUT
{
	float4 Position : SV_POSITION;
};

PS_FADER_INPUT vs_fader( COMMON_POS_NORM_UV input )
{
	PS_FADER_INPUT output;

	output.Position = input.Position;

	return output;
}

float4 ps_fader( PS_FADER_INPUT input ) : SV_Target
{
	return ObjectColor;
}

technique11 fader
{
	pass fader
	{
		SetVertexShader( CompileShader( vs_4_0, vs_fader() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_fader() ) );
	}
}
