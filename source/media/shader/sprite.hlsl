// ----------------------------------------
// for Sprite
// ----------------------------------------
cbuffer SpriteConstantBuffer : register( b13 )
{
	matrix Transform;
};

PS_FLAT_INPUT vs_sprite( PS_FLAT_INPUT input )
{
	PS_FLAT_INPUT output = input;

	output.Position = mul( input.Position, Transform );

	return output;
}

float4 ps_sprite( PS_FLAT_INPUT input ) : SV_Target
{
	return model_texture.Sample( texture_sampler, input.TexCoord ) * input.Color;
}

float4 ps_sprite_add( PS_FLAT_INPUT input ) : SV_Target
{
	return model_texture.Sample( texture_sampler, input.TexCoord ) + input.Color;
}

technique11 sprite
{
	pass main
	{
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( NoDepthTest, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_sprite() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_sprite() ) );
	}
}

technique11 sprite_add
{
	pass main
	{
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( NoWriteDepth, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_sprite() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_sprite_add() ) );
	}
}