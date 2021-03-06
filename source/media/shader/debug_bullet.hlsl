// ----------------------------------------
// for Bullet debug
// ----------------------------------------
VS_LINE_INPUT vs_debug_bullet( VS_LINE_INPUT input )
{
	VS_LINE_INPUT output;

	output.Position = common_vp_pos( input.Position );
	output.Color = input.Color;

	return output;
}

float4 ps_debug_bullet( VS_LINE_INPUT input ) : SV_Target
{
	return float4( input.Color.x, input.Color.y, input.Color.z, 0.75f );
}

technique11 bullet
{
	pass main
	{
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );
		// SetDepthStencilState( NoDepthTest, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_debug_bullet() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_debug_bullet() ) );
	}
}