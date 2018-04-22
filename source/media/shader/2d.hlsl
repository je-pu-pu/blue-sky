// ----------------------------------------
// for 2D ( Fader, debug ウィンドウで使用 )
// ----------------------------------------
PS_FLAT_INPUT vs_2d( VS_INPUT input )
{
	PS_FLAT_INPUT output = ( PS_FLAT_INPUT ) 0;

	output.Position = mul( input.Position, World );
	output.TexCoord = input.TexCoord;

	return output;
}

float4 ps_2d( PS_FLAT_INPUT input ) : SV_Target
{
	return model_texture.Sample( texture_sampler, input.TexCoord ) + ObjectColor; // + ObjectAdditionalColor;
}

technique11 main2d
{
	pass main
	{
		SetVertexShader( CompileShader( vs_4_0, vs_2d() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_2d() ) );
	}
}