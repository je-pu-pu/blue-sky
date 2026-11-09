// ----------------------------------------
// for Sprite
// ----------------------------------------

Texture2D sprite_texture : register( t0 );
Texture2DMS< float4, 8 > sprite_texture_ms : register( t0 );

cbuffer SpriteConstantBuffer : register( b13 )
{
    row_major matrix Transform;
};

COMMON_POS_UV_COLOR vs_sprite( COMMON_POS_UV_COLOR input )
{
	COMMON_POS_UV_COLOR output = input;

	output.Position = mul( input.Position, Transform );

	return output;
}

float4 ps_sprite( COMMON_POS_UV_COLOR input ) : SV_Target
{
	return sprite_texture.Sample( texture_sampler, input.TexCoord ) * input.Color;

	// return sprite_texture.sample[ 0 ][ input.Position.xy ] * input.Color;
	// return sprite_texture.Load( input.Position, 0 ) * input.Color;
}

float4 ps_sprite_add( COMMON_POS_UV_COLOR input ) : SV_Target
{
	return sprite_texture.Sample( texture_sampler, input.TexCoord ) + input.Color;
}

float4 ps_sprite_ms( COMMON_POS_UV_COLOR input ) : SV_Target
{
	float2 texture_size;
	float samples;

	sprite_texture_ms.GetDimensions( texture_size.x, texture_size.y, samples );

	return sprite_texture_ms.Load( input.TexCoord * texture_size, 0 );
}

// スプライト描画
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

		RASTERIZERSTATE = Default;
	}
}

// スプライト描画 ( 加算 )
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

		RASTERIZERSTATE = Default;
	}
}

// スプライト描画 ( マルチサンプリング )
technique11 sprite_ms
{
	pass main
	{
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( NoDepthTest, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_sprite() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_sprite_ms() ) );

		RASTERIZERSTATE = Default;
	}
}

// ----------------------------------------
// テクスチャなし・頂点カラーのみで塗りつぶし矩形を描画
// ----------------------------------------
float4 ps_sprite_color( COMMON_POS_UV_COLOR input ) : SV_Target
{
	return input.Color;
}

technique11 sprite_color
{
	pass main
	{
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( NoDepthTest, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_sprite() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_sprite_color() ) );

		RASTERIZERSTATE = Default;
	}
}

// ----------------------------------------
// 乗算済みアルファのテクスチャ（UI の画像）を描画
//
// - テクスチャは読み込み時に RGB×A 済み（Direct3D11::load_texture_premultiplied）。
//   線形補間しても透明部分の色が縁ににじまない
// - ミップマップは使わない（縮小時にぼけたり、アトラスの隣の部品が混ざったりしないように）
// - 頂点カラーはストレートで渡ってくるので、ここで乗算済みにしてから掛け合わせる
// - アルファも「上に重ねる」合成にする（UI がバックバッファの α を上書きしない）
// ----------------------------------------
SamplerState ui_texture_sampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Clamp;
	AddressV = Clamp;
	AddressW = Clamp;
	MaxLOD = 0;
};

BlendState PremultipliedBlend
{
	BlendEnable[ 0 ] = True;
	SrcBlend = ONE;
	DestBlend = INV_SRC_ALPHA;
	BlendOp = ADD;
	SrcBlendAlpha = ONE;
	DestBlendAlpha = INV_SRC_ALPHA;
	BlendOpAlpha = ADD;
};

float4 ps_sprite_premultiplied( COMMON_POS_UV_COLOR input ) : SV_Target
{
	const float4 tint = float4( input.Color.rgb * input.Color.a, input.Color.a );
	return sprite_texture.Sample( ui_texture_sampler, input.TexCoord ) * tint;
}

technique11 sprite_premultiplied
{
	pass main
	{
		SetBlendState( PremultipliedBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( NoDepthTest, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_sprite() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_sprite_premultiplied() ) );

		RASTERIZERSTATE = Default;
	}
}
