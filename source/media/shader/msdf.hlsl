// ----------------------------------------
// MSDF テキスト描画シェーダー
// ----------------------------------------

Texture2D msdf_atlas : register( t0 );

SamplerState msdf_sampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Clamp;
	AddressV = Clamp;
};

cbuffer MsdfTextConstantBuffer : register( b3 )
{
	float4 TextColor;
	float4 OutlineColor;
	float OutlineWidth;		// アウトライン幅 ( スクリーンピクセル単位, 0 = なし )
	float PxRange;			// MSDF 距離フィールド範囲 ( texel 単位 )
	float AtlasTexelSize;	// 1.0 / アトラスサイズ
};

// sprite.hlsl で定義済みの SpriteConstantBuffer ( b13 ) の Transform を再利用

struct MSDF_VS_INPUT
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
	float4 Color    : COLOR0;
};

struct MSDF_PS_INPUT
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
	float4 Color    : COLOR0;
};

float median( float r, float g, float b )
{
	return max( min( r, g ), min( max( r, g ), b ) );
}

float calc_screen_px_range( float2 tex_coord )
{
	float2 unit_range = float2( PxRange * AtlasTexelSize, PxRange * AtlasTexelSize );
	float2 screen_tex_size = float2( 1.0, 1.0 ) / fwidth( tex_coord );
	return max( 0.5 * dot( unit_range, abs( screen_tex_size ) ), 1.0 );
}

MSDF_PS_INPUT vs_msdf( MSDF_VS_INPUT input )
{
	MSDF_PS_INPUT output;

	output.Position = mul( input.Position, Transform );
	output.TexCoord = input.TexCoord;
	output.Color = input.Color;

	return output;
}

float4 ps_msdf( MSDF_PS_INPUT input ) : SV_Target
{
	float3 s = msdf_atlas.Sample( msdf_sampler, input.TexCoord ).rgb;
	float dist = median( s.r, s.g, s.b );

	float spr = calc_screen_px_range( input.TexCoord );
	float screen_px_dist = spr * ( dist - 0.5 );
	float body_alpha = clamp( screen_px_dist + 0.5, 0.0, 1.0 );

	if ( OutlineWidth > 0.0 )
	{
		// OutlineWidth はスクリーンピクセル単位
		// screen_px_dist はグリフエッジからのスクリーンピクセル距離（外側が負）
		float outline_alpha = clamp( screen_px_dist + OutlineWidth + 0.5, 0.0, 1.0 );

		// SDF 範囲境界でのアーティファクト防止（dist=0 で alpha=0 に）
		float boundary_mask = clamp( spr * dist, 0.0, 1.0 );
		outline_alpha = min( outline_alpha, boundary_mask );

		float4 color = lerp( OutlineColor, TextColor, body_alpha );
		color.a *= outline_alpha;

		return color;
	}
	else
	{
		float4 color = TextColor;
		color.a *= body_alpha;

		return color;
	}
}

technique11 msdf_text
{
	pass main
	{
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( NoDepthTest, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_msdf() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_msdf() ) );

		RASTERIZERSTATE = Default;
	}
}
