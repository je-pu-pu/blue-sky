static const int ShadowMapCascadeLevels = 3;
static const float Pi = 3.14159265f;
static const uint MaxBones = 100;

// model
Texture2D model_texture : register( t0 );		/// モデルのテクスチャ
Texture2D shadow_texture : register( t1 );		/// シャドウマップ
Texture2D paper_texture : register( t2 );		/// 紙の質感テクスチャ
Texture2D matcap_texture : register( t3 );		/// Matcap テクスチャ
Texture2D displacement_texture : register( t4 );/// ディスプレイスメントマッピング用テクスチャ
Texture2D normal_texture : register( t5 );		/// ノーマルマッピング用テクスチャ

// line
Texture2D line_texture : register( t0 );		/// 線のテクスチャ

// canvas
Texture2D pen_texture : register( t1 );			/// ペンのテクスチャ

SamplerState texture_sampler
{
	Filter = ANISOTROPIC;
    AddressU = Clamp;
    AddressV = Clamp;
    AddressW = Clamp;
	ComparisonFunc = NEVER;
};

SamplerState wrap_texture_sampler
{
	Filter = ANISOTROPIC;
    AddressU = Wrap;
    AddressV = Wrap;
    AddressW = Wrap;
	ComparisonFunc = NEVER;
};

SamplerState u_wrap_texture_sampler
{
	Filter = ANISOTROPIC;
    AddressU = Wrap;
    AddressV = Clamp;
    AddressW = Clamp;
	ComparisonFunc = NEVER;
};

SamplerState shadow_texture_sampler
{
	Filter = ANISOTROPIC;
    AddressU = BORDER;
    AddressV = BORDER;
    AddressW = BORDER;
	BorderColor = float4( 1.f, 1.f, 1.f, 1.f );
	ComparisonFunc = NEVER;
	MaxAnisotropy = 2.f;
};

BlendState NoBlend
{
	BlendEnable[ 0 ] = True;
};

BlendState Blend
{
	BlendEnable[ 0 ] = True;

	// SrcBlendAlpha = SRC_ALPHA;
	SrcBlend = SRC_ALPHA;
	DestBlend = INV_SRC_ALPHA;
	// AlphaToCoverageEnable = True;
};

DepthStencilState NoWriteDepth
{
	// DepthEnable = True;
	DepthWriteMask = ZERO;
};

DepthStencilState WriteDepth
{
	// DepthEnable = True;
	DepthWriteMask = ALL;
};

DepthStencilState NoDepthTest
{
	DepthEnable = False;
};

RasterizerState Default
{
	CullMode = BACK;
	// DepthClipEnable = False;
	MultisampleEnable = True;
};

RasterizerState WireframeRasterizerState
{
	FILLMODE = WIREFRAME;
};

RasterizerState Shadow
{
	CullMode = NONE;
	SlopeScaledDepthBias = 2.f;
};

cbuffer GameConstantBuffer : register( b0 )
{
	float ScreenWidth;
	float ScreenHeight;
};

cbuffer FrameConstantBuffer : register( b1 )
{
	/// @todo ViewProejction としてまとめる？
	matrix View;			// ビュー変換行列
	matrix Projection;		// プロジェクション変換行列
	float4 Light;			// 光源の向き ( 正規化済み ) 
	float Time;				// シーン開始からの経過秒数
	uint TimeBeat;			// 現在の音楽の BPM ?
	float TessFactor;		// テッセレーションの分割数
};

cbuffer ObjectConstantBuffer : register( b2 )
{
	matrix World;			// ワールド変換行列
	float4 ObjectColor;		// オブジェクトの色
};

cbuffer FrameDrawingConstantBuffer : register( b4 )
{
	float4 ShadowColor;
	float4 ShadowPaperColor;
	float DrawingAccent;
	uint LineType;
}

cbuffer BoneConstantBuffer : register( b5 )
{
    matrix BoneMatrix[ MaxBones ];
};

cbuffer ShadowMapConstantBuffer : register( b10 )
{
	matrix ShadowViewProjection[ ShadowMapCascadeLevels ];
	float4 ShadowMapViewDepthPerCascadeLevel;
};

struct VS_SKIN_INPUT
{
	float4 Position : POSITION;
	float3 Normal   : NORMAL0;
	float2 TexCoord : TEXCOORD0;
	uint4  Bone     : BONE;
	float4 Weight   : WEIGHT;
};

struct VS_LINE_INPUT
{
	float4 Position : SV_POSITION;
	float4 Color    : COLOR0;
};

struct GS_LINE_INPUT
{
	float4 Position : SV_POSITION;
	float4 Color    : COLOR0;
};

struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float3 Normal   : NORMAL0;
	float2 TexCoord : TEXCOORD0;
	float4 Color    : COLOR0;
};

struct PS_FLAT_INPUT
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
	float4 Color    : COLOR0;
};

struct PS_SHADOW_INPUT
{
	float4 Position : SV_POSITION;
	float3 Normal   : NORMAL0;
	float4 ShadowTexCoord[ ShadowMapCascadeLevels ] : TEXCOORD0;
	float2 TexCoord : TEXCOORD4;
	float Depth : DEPTH;
};

#include "common.hlsl"
#include "matcap.hlsl"
#include "drawing_line.hlsl"

PS_INPUT vs_main( COMMON_POS_NORM_UV input, uint vertex_id : SV_VertexID )
{
	PS_INPUT output;

	output.Position = common_wvp_pos( input.Position );
	output.Normal = common_w_norm( input.Normal );
	output.TexCoord = input.TexCoord;
	output.Color = ObjectColor;

	/*
	output.Color = 
		( vertex_id + 8  ) % 32 / 32.f,
		( vertex_id + 16 ) % 32 / 32.f,
		( vertex_id + 24 ) % 32 / 32.f,
		1.f
	);
	*/

	return output;
}

PS_INPUT vs_skin( VS_SKIN_INPUT input )
{
	PS_INPUT output;

	output.Position = common_skinning_pos( input.Position, input.Bone, input.Weight );
	output.Position = common_wvp_pos( output.Position );

	output.Normal = common_skinning_norm( input.Normal, input.Bone, input.Weight );
	output.Normal = common_w_norm( output.Normal );
	
	output.TexCoord = input.TexCoord;

	output.Color = ObjectColor;

	return output;
}

PS_FLAT_INPUT vs_flat( COMMON_POS_NORM_UV input )
{
	PS_FLAT_INPUT output;

	output.Position = common_wvp_pos( input.Position );
	output.TexCoord = input.TexCoord;
	output.Color = ObjectColor;

	return output;
}

/**
 * 紙の質感テクスチャをサンプリングする
 *
 */
float4 sample_paper_texture( float2 xy )
{
	const float Scale = 0.5f; // 0.75f + DrawingAccent * 0.25f;
			
	const float PaperTextureWidth  = 512.f * Scale;
	const float PaperTextureHeight = 512.f * Scale;

	float2 uv = float2(
		xy.x / ScreenWidth  * ( ScreenWidth  / PaperTextureWidth  ),
		xy.y / ScreenHeight * ( ScreenHeight / PaperTextureHeight )
	);

	// アクセントを加える
	uv += float2( TimeBeat * 0.2f, 0.f );
			
	return paper_texture.Sample( wrap_texture_sampler, uv );
}

float4 ps_with_paper_common( float3 position, float2 uv, float diffuse )
{
	float4 shadow = float4( 1.f, 1.f, 1.f, 1.f );

	{
		// 影
		const float4 shadow_color = ShadowColor;
		shadow = float4( 1.f, 1.f, 1.f, 1.f ) * ( 1.f - shadow_color.a ) + shadow_color * shadow_color.a;
		shadow.a = 1.f;
		
		// 紙の質感を追加する
		if ( true )
		{
			const float3 shadow_paper_color = ( float3 ) ShadowPaperColor;

			float4 paper = sample_paper_texture( float2( position.x + position.z * 0.01f, position.y ) );
			shadow.rgb = shadow_paper_color * ( 1.f - paper.a ) + shadow.rgb * paper.a;
		}

		const float shadow_rate = ( 1.f - diffuse );
		shadow.rgb = float3( 1.f, 1.f, 1.f ) * ( 1.f - shadow_rate ) + shadow.rgb * shadow_rate;
	}

	static const float4 cascade_colors[ 4 ] = 
	{
		float4 ( 1.2f, 1.0f, 1.0f, 1.0f ),
		float4 ( 1.0f, 1.2f, 1.0f, 1.0f ),
		float4 ( 1.0f, 1.0f, 1.2f, 1.0f ),
		float4 ( 1.2f, 1.2f, 0.0f, 1.0f ),
	};

	return model_texture.Sample( wrap_texture_sampler, uv ) * shadow; // * cascade_colors[ cascade_index ];
}

float4 ps_flat( PS_FLAT_INPUT input ) : SV_Target
{
	float4 output = model_texture.Sample( texture_sampler, input.TexCoord ) * input.Color;

	// clip( output.a - 0.0001f );

	return output;
}

float4 ps_flat_with_flicker( PS_FLAT_INPUT input ) : SV_Target
{
	return model_texture.Sample( texture_sampler, input.TexCoord + float2( sin( input.TexCoord.y * 50.f + TimeBeat ) * 0.01f, 0.f ) ) + input.Color;
}

float4 ps_main_wrap( PS_INPUT input ) : SV_Target
{
	// return common_sample_matcap( normalize( common_v_norm( input.Normal ) ) );
	// return float4( input.Normal * 0.5f + float3( 0.5f, 0.5f, 0.5f ), 1.f );

	float diffuse = ( 1.f - ( dot( input.Normal, ( float3 ) Light ) * 0.5f + 0.5f ) );
	return ps_with_paper_common( ( float3 ) input.Position, input.TexCoord, diffuse );

	return model_texture.Sample( wrap_texture_sampler, input.TexCoord ); // /* + input.Color */ * float4( input.Normal, 1.f );
}

/**
 * 陰影なし、スキニングあり、影なしのシェーダー
 */
float4 ps_skin_wrap_flat( PS_INPUT input ) : SV_Target
{
	return model_texture.Sample( wrap_texture_sampler, input.TexCoord );
}

/**
 * 陰影なし、スキニングあり、影なし + 紙の質感のシェーダー
 */
float4 ps_skin_wrap_flat_paper( PS_INPUT input ) : SV_Target
{
	float4 model_color = model_texture.Sample( wrap_texture_sampler, input.TexCoord );
	float4 paper_color = sample_paper_texture( input.Position.xy );
	float paper_factor = 0.25f;

	return float4( lerp( model_color.rgb, paper_color.rgb, float3( 1.f - paper_color.a, 1.f - paper_color.a , 1.f - paper_color.a ) * paper_factor ), model_color.a );
}

float4 ps_main_wrap_flat( PS_FLAT_INPUT input ) : SV_Target
{
	return model_texture.Sample( wrap_texture_sampler, input.TexCoord );
}

/**
 * 
 *
 *
 */
PS_SHADOW_INPUT vs_with_shadow( COMMON_POS_NORM_UV input )
{
	PS_SHADOW_INPUT output;

	output.Position = common_wvp_pos( input.Position );
	output.Normal = common_w_norm( input.Normal );
	output.TexCoord = input.TexCoord;
	
	[unroll]
	for ( int n = 0; n < ShadowMapCascadeLevels; n++ )
	{
		output.ShadowTexCoord[ n ] = common_shadow_texcoord( input.Position, n );
	}

	output.Depth = common_wv_pos( input.Position ).z;

	return output;
}

/**
 * 
 *
 *
 */
PS_SHADOW_INPUT vs_skin_with_shadow( VS_SKIN_INPUT input )
{
	PS_SHADOW_INPUT output;

	output.Position = common_skinning_pos( input.Position, input.Bone, input.Weight );
	output.Position = common_wvp_pos( output.Position );
	
	output.Normal = common_skinning_norm( input.Normal, input.Bone, input.Weight );
	output.Normal = common_w_norm( output.Normal );
	
	output.TexCoord = input.TexCoord;
	
	[unroll]
	for ( int n = 0; n < ShadowMapCascadeLevels; n++ )
	{
		output.ShadowTexCoord[ n ] = common_skinning_pos( input.Position, input.Bone, input.Weight );
		output.ShadowTexCoord[ n ] = common_shadow_texcoord( output.ShadowTexCoord[ n ], n );
	}

	output.Depth = common_wv_pos( input.Position ).z;

	return output;
}

/**
 * 
 *
 */
float4 ps_with_shadow( PS_SHADOW_INPUT input ) : SV_Target
{
	const bool is_shadow = common_sample_is_shadow( input.ShadowTexCoord, input.Depth );
	
	float diffuse = common_diffuse( input.Normal );

	if ( is_shadow || diffuse <= 0.5f )
	{
		diffuse *= 0.5f;
	}

	diffuse = max( 0.f, diffuse );

	return ps_with_paper_common( ( float3 ) input.Position, input.TexCoord, diffuse );

	// float4 output = ps_with_paper_common( ( float3 ) input.Position, input.TexCoord, diffuse );
	// clip( output.a - 0.0001f );
	// return output;
}

float4 ps_with_shadow_debug_cascade_level( PS_SHADOW_INPUT input ) : SV_Target
{
	const bool is_shadow = common_sample_is_shadow( input.ShadowTexCoord, input.Depth );

	if ( is_shadow )
	{
		const int cascade_index = common_shadow_cascade_index( input.Depth );

		float4 colors[ 4 ] = {
			{ 1.f, 0.f, 0.f, 1.f },
			{ 0.f, 1.f, 0.f, 1.f },
			{ 0.f, 0.f, 1.f, 1.f },
			{ 1.f, 1.f, 0.f, 1.f }
		};

		return 0.5 * colors[ cascade_index ] + 0.5 * model_texture.Sample( wrap_texture_sampler, input.TexCoord );
	}

	return model_texture.Sample( wrap_texture_sampler, input.TexCoord );
}

/**
 *
 */
float4 ps_with_shadow_debug_simple( PS_SHADOW_INPUT input ) : SV_Target
{
	// return common_sample_matcap( normalize( common_v_norm( input.Normal ) ) );

	const float diffuse = common_diffuse( input.Normal );
	return float4( diffuse, diffuse, diffuse, 1.f );

	const bool is_shadow = common_sample_is_shadow( input.ShadowTexCoord, input.Depth );

	if ( is_shadow )
	{
		return float4( diffuse * 0.5f, diffuse * 0.5f, diffuse * 0.5f, 1.f );
	}
}


/**
 * @todo 以下の命名規則にまとめる
 *
 * 1. 陰影を付ける場合は "shade", 付けない場合は "flat"
 * 2. スキニングを行う場合は"_skin" を付加
 * 3. 影を付ける場合は "_shadow" を付加
 *
 */

// シェーディングあり・スキニングなし
/// @todo technique11 shade 
technique11 main
{
	pass main
    {
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

        SetVertexShader( CompileShader( vs_4_0, vs_main() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, ps_main_wrap() ) );

		RASTERIZERSTATE = Default;
    }
}

// シェーディングあり・スキニングあり
/// @todo technique11 shade_skin
technique11 skin
{
	pass main
    {
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

        SetVertexShader( CompileShader( vs_4_0, vs_skin() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, ps_main_wrap() ) );

		RASTERIZERSTATE = Default;
    }
}

// シェーディングなし・スキニングなし
technique11 flat
{
	pass main
    {
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

        SetVertexShader( CompileShader( vs_4_0, vs_flat() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, ps_main_wrap_flat() ) );

		RASTERIZERSTATE = Default;
    }

#if 1
	pass debug_line
    {
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( DebugLineDepthStencilState, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_common_wvp_pos_norm_uv_to_pos() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_common_debug_line_pos() ) );

		RASTERIZERSTATE = WireframeRasterizerState;
    }
#endif
}

// シェーディングなし・スキニングあり
technique11 flat_skin
{
	pass main
    {
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

        SetVertexShader( CompileShader( vs_4_0, vs_skin() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, ps_skin_wrap_flat() ) );
		// SetPixelShader( CompileShader( ps_4_0, ps_skin_wrap_flat_paper() ) );

		RASTERIZERSTATE = Default;
    }
}

// シェーディングあり・スキニングなし・シャドウあり
/// @todo technique11 shade_shadow
technique11 main_with_shadow
{
	
	pass main
	{
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_with_shadow() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_with_shadow() ) );
		// SetPixelShader( CompileShader( ps_4_0, ps_with_shadow_debug_simple() ) );
		// SetPixelShader( CompileShader( ps_4_0, ps_with_shadow_debug_cascade_level() ) );

		RASTERIZERSTATE = Default;
	}
}

// シェーディングあり・スキニングあり・シャドウあり
/// @todo technique11 shade_skin_shadow
technique11 skin_with_shadow
{
	pass main
    {
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

        SetVertexShader( CompileShader( vs_4_0, vs_skin_with_shadow() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, ps_with_shadow() ) );
		// SetPixelShader( CompileShader( ps_4_0, ps_with_shadow_debug_simple() ) );
		// SetPixelShader( CompileShader( ps_4_0, ps_with_shadow_debug_cascade_level() ) );

		RASTERIZERSTATE = Default;
    }
}

#include "tessellation.hlsl"
#include "shadow_map.hlsl"

#include "sky_box.hlsl"
#include "ground.hlsl"
#include "billboard.hlsl"

#include "2d.hlsl"
#include "sprite.hlsl"
#include "fader.hlsl"

#include "canvas.hlsl"

#include "debug_axis.hlsl"
#include "debug_bullet.hlsl"

#include "post_effect/common.hlsl"
#include "post_effect/default.hlsl"
#include "post_effect/chromatic_aberration.hlsl"
#include "post_effect/hand_drawing.hlsl"
