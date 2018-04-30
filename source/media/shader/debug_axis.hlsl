// ----------------------------------------
// for debug axis
// ----------------------------------------
VS_LINE_INPUT vs_debug_axis( VS_LINE_INPUT input )
{
	input.Position = common_wvp_pos( input.Position );

	return input;
}

float4 ps_debug_axis( VS_LINE_INPUT input ) : SV_Target
{
	return float4( input.Color.x, input.Color.y, input.Color.z, 1.f );
}

technique11 simple_line
{
	pass main
	{
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( NoDepthTest, 0xFFFFFFFF );
		// SetDepthStencilState( WriteDepth, 0xFFFFFFFF );
		
		SetVertexShader( CompileShader( vs_4_0, vs_debug_axis() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_debug_axis() ) );
	}
}