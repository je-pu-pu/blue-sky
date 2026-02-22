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
	float OutlineWidth;		// 0.0 ~ 0.5
	float Smoothing;		// アンチエイリアス幅
};

cbuffer SpriteConstantBuffer : register( b13 )
{
	row_major matrix Transform;
};

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

	// 本体
	float body_alpha = smoothstep( 0.5 - Smoothing, 0.5 + Smoothing, dist );

	if ( OutlineWidth > 0.0 )
	{
		// 縁取り
		float outline_edge = 0.5 - OutlineWidth;
		float outline_alpha = smoothstep( outline_edge - Smoothing, outline_edge + Smoothing, dist );

		float4 color = lerp( OutlineColor, TextColor * input.Color, body_alpha );
		color.a *= outline_alpha;

		return color;
	}
	else
	{
		float4 color = TextColor * input.Color;
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
