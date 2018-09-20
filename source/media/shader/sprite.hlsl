// ----------------------------------------
// for Sprite
// ----------------------------------------

// Texture2D sprite_texture : register( t0 );
Texture2DMS< float4, 8 > sprite_texture : register( t0 );

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
	float2 texture_size;
	float samples;

	sprite_texture.GetDimensions( texture_size.x, texture_size.y, samples );

	return sprite_texture.Load( int3( input.TexCoord * texture_size, 0 ), 0 );

	// return sprite_texture.Sample( texture_sampler, input.TexCoord ) * input.Color;
	// return sprite_texture.sample[ 0 ][ input.Position.xy ] * input.Color;
	// return sprite_texture.Load( input.Position, 0 ) * input.Color;
}

float4 ps_sprite_add( PS_FLAT_INPUT input ) : SV_Target
{
	return float4( 1, 0, 0, 0 );

	// return sprite_texture.Load( int3( 0, 0, 0 ) );

	// return sprite_texture.Sample( texture_sampler, input.TexCoord ) + input.Color;
	// return sprite_texture.sample[ 7 ][ input.TexCoord ] + input.Color;
	// return sprite_texture.Load( input.TexCoord, 0 ) * input.Color;
}

technique11 sprite
{
	pass main
	{
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( NoDepthTest, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_sprite() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
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
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_sprite_add() ) );
	}
}