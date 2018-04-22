// ----------------------------------------
// for debug axis
// ----------------------------------------
VS_LINE_INPUT vs_line( VS_LINE_INPUT input )
{
	input.Position = mul( input.Position, World );
    input.Position = mul( input.Position, View );
    input.Position = mul( input.Position, Projection );

	return input;
}

float4 ps_line( VS_LINE_INPUT input ) : SV_Target
{
	return float4( input.Color.x, input.Color.y, input.Color.z, 1.f );
}

technique11 simple_line
{
	pass main
	{
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( NoDepthTest, 0xFFFFFFFF );
		
		SetVertexShader( CompileShader( vs_4_0, vs_line() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_line() ) );
	}
}