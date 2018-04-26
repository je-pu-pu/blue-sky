static const int ShadowMapCascadeLevels = 3;
static const float Pi = 3.14159265f;
static const uint MaxBones = 100;

Texture2D model_texture : register( t0 );		/// モデルのテクスチャ
Texture2D line_texture : register( t0 );		/// 線のテクスチャ
Texture2D shadow_texture : register( t1 );		/// シャドウマップ
Texture2D paper_texture : register( t2 );		/// 紙の質感テクスチャ
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

BlendState CanvasPenBlend
{
	BlendEnable[ 0 ] = True;
	
	SrcBlend = SRC_ALPHA;
	DestBlend = INV_SRC_ALPHA;
	AlphaToCoverageEnable = False;
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

RasterizerState WireFrame
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
	/// @todo ViewProejction としてまとめる
	matrix View;			// ビュー変換行列
	matrix Projection;		// プロジェクション変換行列
	float4 Light;			// 光源の向き ( 正規化済み ) 
	float Time;				// シーン開始からの経過秒数
	uint TimeBeat;			// 現在の音楽の BPM ?
};

cbuffer ObjectConstantBuffer : register( b2 )
{
	matrix World;
	float4 ObjectColor;
};

cbuffer ObjectExtensionConstantBuffer : register( b3 )
{
	float4 ObjectAdditionalColor;
}

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

cbuffer ShadowMapBuffer : register( b10 )
{
	matrix ShadowViewProjection[ ShadowMapCascadeLevels ];
	float4 ShadowMapViewDepthPerCascadeLevel;
};

struct VS_INPUT
{
	float4 Position : SV_POSITION;
	float3 Normal   : NORMAL0;
	float2 TexCoord : TEXCOORD0;
};

struct GS_2D_INPUT
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
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
	float Depth : TEXCOORD5;
};

#include "common.hlsl"
#include "drawing_line.hlsl"

PS_INPUT vs_main( VS_INPUT input, uint vertex_id : SV_VertexID )
{
	PS_INPUT output;

	output.Position = mul( input.Position, World );
    output.Position = mul( output.Position, View );
    output.Position = mul( output.Position, Projection );
	
	output.Normal = mul( input.Normal, ( float3x3 ) World );
	
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
	PS_INPUT output = ( PS_INPUT ) 0;

	output.Position +=
		mul( input.Position, BoneMatrix[ input.Bone.x ] ) * input.Weight.x +
		mul( input.Position, BoneMatrix[ input.Bone.y ] ) * input.Weight.y +
		mul( input.Position, BoneMatrix[ input.Bone.z ] ) * input.Weight.z +
		mul( input.Position, BoneMatrix[ input.Bone.w ] ) * input.Weight.w;
	
	output.Normal +=
		mul( input.Normal, ( float3x3 ) BoneMatrix[ input.Bone.x ] ) * input.Weight.x +
		mul( input.Normal, ( float3x3 ) BoneMatrix[ input.Bone.y ] ) * input.Weight.y +
		mul( input.Normal, ( float3x3 ) BoneMatrix[ input.Bone.z ] ) * input.Weight.z +
		mul( input.Normal, ( float3x3 ) BoneMatrix[ input.Bone.w ] ) * input.Weight.w;

	output.Position = mul( output.Position, World );
    output.Position = mul( output.Position, View );
    output.Position = mul( output.Position, Projection );

	output.Normal = mul( output.Normal, ( float3x3 ) World );
	
	output.TexCoord = input.TexCoord;
	output.Color = ObjectColor;

	return output;
}

PS_FLAT_INPUT vs_flat( VS_INPUT input )
{
	PS_FLAT_INPUT output;

	output.Position = mul( input.Position, World );
    output.Position = mul( output.Position, View );
    output.Position = mul( output.Position, Projection );
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
	return model_texture.Sample( texture_sampler, input.TexCoord ) * input.Color;
}

float4 ps_flat_with_flicker( PS_FLAT_INPUT input ) : SV_Target
{
	return model_texture.Sample( texture_sampler, input.TexCoord + float2( sin( input.TexCoord.y * 50.f + TimeBeat ) * 0.01f, 0.f ) ) + input.Color;
}

float4 ps_main_wrap( PS_INPUT input ) : SV_Target
{
	// return float4( input.Normal * 0.5f + float3( 0.5f, 0.5f, 0.5f ), 1.f );

	float diffuse = ( 1.f - ( dot( input.Normal, ( float3 ) Light ) * 0.5f + 0.5f ) );
	return ps_with_paper_common( ( float3 ) input.Position, input.TexCoord, diffuse );

	return model_texture.Sample( wrap_texture_sampler, input.TexCoord ); // /* + input.Color */ * float4( input.Normal, 1.f );
}

float4 ps_skin_wrap_flat( PS_INPUT input ) : SV_Target
{
	return model_texture.Sample( wrap_texture_sampler, input.TexCoord );
}

float4 ps_main_wrap_flat( PS_FLAT_INPUT input ) : SV_Target
{
	return model_texture.Sample( wrap_texture_sampler, input.TexCoord );
}

float4 ps_debug( PS_INPUT input ) : SV_Target
{
	return float4( 1.f, 0.f, 0.f, 0.25f );
}

struct VS_CANVAS_INPUT
{
	float4 Position : SV_POSITION;
	float  Pressure : PRESSURE;
	float4 Color    : COLOR0;
};

struct GS_CANVAS_INPUT
{
	float4 Position : SV_POSITION;
	float  Pressure : PRESSURE;
	float4 Color    : COLOR0;
};

/**
 * Canvas
 *
 */
GS_CANVAS_INPUT vs_canvas( VS_CANVAS_INPUT input, uint vertex_id : SV_VertexID )
{
	GS_CANVAS_INPUT output;

	output.Position = mul( input.Position, World );
	output.Position = mul( output.Position, View );
    output.Position = mul( output.Position, Projection );
	output.Pressure = input.Pressure;
	output.Color = input.Color;

	// 色変動
	if ( true )
	{
		static const float color_random_range = 0.02f;
		output.Color.r += ( ( ( uint( Time * 5 ) + vertex_id ) % 8 ) / 4.f - 1.f ) * color_random_range;
		output.Color.g += ( ( ( uint( Time * 15 ) + vertex_id ) % 8 ) / 4.f - 1.f ) * color_random_range;
		output.Color.b += ( ( ( uint( Time * 25 ) + vertex_id ) % 8 ) / 4.f - 1.f ) * color_random_range;
	}

	// 位置変動
	if ( true )
	{
		const float mx = ( ( vertex_id + 8  ) % 10 ) + 1;
		const float my = ( ( vertex_id + 10 ) % 28 ) + 1;
		const float mz = ( ( vertex_id + 15 ) % 15 ) + 1;

		output.Position.x += cos( vertex_id + Time / mx ) * 0.002f;
		output.Position.y += sin( vertex_id + Time / my ) * 0.002f;
		output.Position.z += sin( vertex_id + Time / mz ) * 0.002f;
	}

	return output;
}

void add_pen_point( inout TriangleStream<PS_FLAT_INPUT> TriStream , in GS_CANVAS_INPUT input, uint primitive_id )
{
	const float screen_width = ScreenWidth;
	const float screen_height = ScreenHeight;
	const float screen_ratio = ( screen_height / screen_width );

	const float l = 0.1f * input.Pressure;
	const float hw = l * 0.5f * screen_ratio;
	const float hh = l * 0.5f;
	
	PS_FLAT_INPUT output[ 4 ];
	
	// left top
	output[ 0 ].Position = input.Position + float4( -hw, -hh, 0.f, 0.f );
	output[ 0 ].TexCoord.xy = float2( 0.f, 1.f );

	// right top
	output[ 1 ].Position = input.Position + float4( +hw, -hh, 0.f, 0.f );
	output[ 1 ].TexCoord.xy = float2( 1.f, 1.f );

	// left bottom
	output[ 2 ].Position = input.Position + float4( -hw, +hh, 0.f, 0.f );
	output[ 2 ].TexCoord.xy = float2( 0.f, 0.f );

	// right bottom
	output[ 3 ].Position = input.Position + float4( +hw, +hh, 0.f, 0.f );
	output[ 3 ].TexCoord.xy = float2( 1.f, 0.f );

	for ( uint y = 0; y < 4; ++y )
	{
		output[ y ].Color = input.Color;
	}

	TriStream.Append( output[ 0 ] );
	TriStream.Append( output[ 2 ] );
	TriStream.Append( output[ 1 ] );
	TriStream.RestartStrip();

	TriStream.Append( output[ 1 ] );
	TriStream.Append( output[ 2 ] );
	TriStream.Append( output[ 3 ] );
	TriStream.RestartStrip();
}

/**
 * Canvas
 *
 */
[maxvertexcount(6)]
void gs_canvas( point GS_CANVAS_INPUT input[ 1 ], inout TriangleStream<PS_FLAT_INPUT> TriStream, uint primitive_id : SV_PrimitiveID )
{
	add_pen_point( TriStream, input[ 0 ], primitive_id );
}

struct PS_CANVAS_OUTPUT
{
	float4 Color : SV_Target;
	float Depth  : SV_Depth;
};

/**
 * Canvas
 *
 */
PS_CANVAS_OUTPUT ps_canvas( PS_FLAT_INPUT input )
{
	PS_CANVAS_OUTPUT output = ( PS_CANVAS_OUTPUT ) 0;
	
	// output.Color = pen_texture.Sample( texture_sampler, input.TexCoord ) * input.Color;
	output.Color = pen_texture.Sample( texture_sampler, input.TexCoord ) + ( ( ( input.Color * float4( 2.f, 2.f, 2.f, 0.f ) ) - float4( 1.f, 1.f, 1.f, 0.f ) ) * float4( 0.5f, 0.5f, 0.5f, 1.f ) );
	// output.Color = input.Color;

	// output.Depth = input.Position.z * ( 1 - output.Color.a );

	if ( output.Color.a <= 0.95f )
	{
		discard;
	}

	output.Depth = input.Position.z;

	return output;
}

// ----------------------------------------
// shaders
// ----------------------------------------

/**
 * シャドウマップへのレンダリング
 *
 */
float4 vs_shadow_map( VS_INPUT input ) : SV_POSITION
{
	float4 output;

	output = mul( input.Position, World );
    output = mul( output, ShadowViewProjection[ 0 ] );

	return output;
}

/**
 * シャドウマップへのスキンメッシュのレンダリング
 *
 */
float4 vs_shadow_map_skin( VS_SKIN_INPUT input ) : SV_POSITION
{
	float4 output;

	output =
		mul( input.Position, BoneMatrix[ input.Bone.x ] ) * input.Weight.x +
		mul( input.Position, BoneMatrix[ input.Bone.y ] ) * input.Weight.y +
		mul( input.Position, BoneMatrix[ input.Bone.z ] ) * input.Weight.z +
		mul( input.Position, BoneMatrix[ input.Bone.w ] ) * input.Weight.w;

	output = mul( output, World );
    output = mul( output, ShadowViewProjection[ 0 ] );

	return output;
}

/**
 * 
 *
 *
 */
PS_SHADOW_INPUT vs_with_shadow( VS_INPUT input )
{
	PS_SHADOW_INPUT output;

	output.Position = mul( input.Position, World );
    output.Position = mul( output.Position, View );
    output.Position = mul( output.Position, Projection );
	
	output.Normal = normalize( mul( input.Normal, ( float3x3 ) World ) );
	
	output.TexCoord = input.TexCoord;
	
	[unroll]
	for ( int n = 0; n < ShadowMapCascadeLevels; n++ )
	{
		output.ShadowTexCoord[ n ] = mul( input.Position, World );
		output.ShadowTexCoord[ n ] = mul( output.ShadowTexCoord[ n ], ShadowViewProjection[ n ] );
		
		output.ShadowTexCoord[ n ] /= output.ShadowTexCoord[ n ].w;
	
		output.ShadowTexCoord[ n ].x = ( output.ShadowTexCoord[ n ].x + 1.f ) / 2.f;
		output.ShadowTexCoord[ n ].y = ( -output.ShadowTexCoord[ n ].y + 1.f ) / 2.f;
		// output.ShadowTexCoord[ n ].z -= 0.00001f;
		// output.ShadowTexCoord[ n ].z -= 0.000005f;
	}

	output.Depth = mul( mul( input.Position, World ), View ).z;

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

	output.Position =
		mul( input.Position, BoneMatrix[ input.Bone.x ] ) * input.Weight.x +
		mul( input.Position, BoneMatrix[ input.Bone.y ] ) * input.Weight.y +
		mul( input.Position, BoneMatrix[ input.Bone.z ] ) * input.Weight.z +
		mul( input.Position, BoneMatrix[ input.Bone.w ] ) * input.Weight.w;

	output.Position = mul( output.Position, World );
    output.Position = mul( output.Position, View );
    output.Position = mul( output.Position, Projection );
	
	output.Normal =
		mul( input.Normal, ( float3x3 ) BoneMatrix[ input.Bone.x ] ) * input.Weight.x +
		mul( input.Normal, ( float3x3 ) BoneMatrix[ input.Bone.y ] ) * input.Weight.y +
		mul( input.Normal, ( float3x3 ) BoneMatrix[ input.Bone.z ] ) * input.Weight.z +
		mul( input.Normal, ( float3x3 ) BoneMatrix[ input.Bone.w ] ) * input.Weight.w;

	output.Normal = normalize( mul( output.Normal, ( float3x3 ) World ) );
	
	output.TexCoord = input.TexCoord;
	
	[unroll]
	for ( int n = 0; n < ShadowMapCascadeLevels; n++ )
	{
		/// @todo 高速化する
		output.ShadowTexCoord[ n ] = 
			mul( input.Position, BoneMatrix[ input.Bone.x ] ) * input.Weight.x +
			mul( input.Position, BoneMatrix[ input.Bone.y ] ) * input.Weight.y +
			mul( input.Position, BoneMatrix[ input.Bone.z ] ) * input.Weight.z +
			mul( input.Position, BoneMatrix[ input.Bone.w ] ) * input.Weight.w;
		output.ShadowTexCoord[ n ] = mul( output.ShadowTexCoord[ n ], World );
		output.ShadowTexCoord[ n ] = mul( output.ShadowTexCoord[ n ], ShadowViewProjection[ n ] );
		
		output.ShadowTexCoord[ n ] /= output.ShadowTexCoord[ n ].w;
	
		output.ShadowTexCoord[ n ].x = ( output.ShadowTexCoord[ n ].x + 1.f ) / 2.f;
		output.ShadowTexCoord[ n ].y = ( -output.ShadowTexCoord[ n ].y + 1.f ) / 2.f;
	}

	output.Depth = mul( mul( input.Position, World ), View ).z;

	return output;
}

/**
 * 
 *
 */
float4 ps_with_shadow( PS_SHADOW_INPUT input ) : SV_Target
{
	const float4 comp = input.Depth > ShadowMapViewDepthPerCascadeLevel;

	float index =
		dot(
			float4(
				ShadowMapCascadeLevels > 0,
				ShadowMapCascadeLevels > 1,
				ShadowMapCascadeLevels > 2,
				ShadowMapCascadeLevels > 3
			),
			comp
		);

	index = min( index, ShadowMapCascadeLevels - 1 );

	const int cascade_index = ( int ) index;

	float4 shadow_tex_coord = input.ShadowTexCoord[ cascade_index ];

	shadow_tex_coord.x /= ( float ) ShadowMapCascadeLevels;
	shadow_tex_coord.x += ( float ) cascade_index / ( float ) ShadowMapCascadeLevels;

	// return float4( input.Normal, 1.f );

	float4 depth = shadow_texture.Sample( shadow_texture_sampler, shadow_tex_coord.xy );

	// const float vsm_variance = depth.y - pow( depth.x, 2 );
	// const float vsm_p = vsm_variance / ( vsm_variance + pow( shadow_tex_coord.z - depth.x, 2 ) );
	// const float vsm_shadow_rate = 1.f - saturate( max( vsm_p, depth.x <= shadow_tex_coord.z ) );

	float diffuse = ( 1.f - ( dot( input.Normal, ( float3 ) Light ) * 0.5f + 0.5f ) );
	
	if ( shadow_tex_coord.z >= depth.x || diffuse <= 0.5f )
	{
		diffuse *= 0.5f;
	}

	diffuse = max( 0.f, diffuse );

	return ps_with_paper_common( ( float3 ) input.Position, input.TexCoord, diffuse );

	// float4 output = ps_with_paper_common( ( float3 ) input.Position, input.TexCoord, diffuse );
	// clip( output.a - 0.0001f );
	// return output;
}

// ----------------------------------------
// main
// ----------------------------------------
technique11 main
{
	pass main
    {
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

        SetVertexShader( CompileShader( vs_4_0, vs_main() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, ps_main_wrap() ) );

		RASTERIZERSTATE = Default;
    }
}

technique11 main_flat
{
	pass main
    {
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

        SetVertexShader( CompileShader( vs_4_0, vs_flat() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, ps_main_wrap_flat() ) );

		RASTERIZERSTATE = Default;
    }
}

technique11 drawing_point
{
	pass main
    {
		SetBlendState( CanvasPenBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

        SetVertexShader( CompileShader( vs_4_0, vs_canvas() ) );
		SetGeometryShader( CompileShader( gs_4_0, gs_canvas() ) );
        SetPixelShader( CompileShader( ps_4_0, ps_canvas() ) );

		RASTERIZERSTATE = Default;
    }
}

// ----------------------------------------
// skin
// ----------------------------------------
technique11 skin
{
	pass main
    {
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

        SetVertexShader( CompileShader( vs_4_0, vs_skin() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, ps_main_wrap() ) );

		RASTERIZERSTATE = Default;
    }
}

technique11 skin_flat
{
	pass main
    {
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

        SetVertexShader( CompileShader( vs_4_0, vs_skin() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, ps_skin_wrap_flat() ) );

		RASTERIZERSTATE = Default;
    }
}

technique11 skin_with_shadow
{
	pass main
    {
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

        SetVertexShader( CompileShader( vs_4_0, vs_skin_with_shadow() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, ps_with_shadow() ) );

		RASTERIZERSTATE = Default;
    }
}

// ----------------------------------------
// for Text
// ----------------------------------------
GS_2D_INPUT vs_text( VS_INPUT input )
{
	GS_2D_INPUT output;
	output.Position = input.Position;
	output.TexCoord = input.TexCoord;

	return output;
}

[maxvertexcount(4)]
void gs_text( triangle GS_2D_INPUT input[3], inout TriangleStream<PS_FLAT_INPUT> TriStream )
{
	PS_FLAT_INPUT output[ 4 ];

	float w = 1.f;
	float h = 1.f;

	output[ 0 ].Position = float4( -w, +h, 0.f, 1.f );
	output[ 0 ].TexCoord = float2( 0.f,  0.f );
	output[ 0 ].Color = float4( 1.f, 0.f, 0.f, 0.f );
	
	output[ 1 ].Position = float4( +w, +h, 0.f, 1.f );
	output[ 1 ].TexCoord = float2( 1.f,  0.f );
	output[ 1 ].Color = float4( 0.f, 1.f, 0.f, 0.f );

	output[ 2 ].Position = float4( -w, -h, 0.f, 1.f );
	output[ 2 ].TexCoord = float2( 0.f,  1.f );
	output[ 2 ].Color = float4( 0.f, 0.f, 1.f, 0.f );

	output[ 3 ].Position = float4( +w, -h, 0.f, 1.f );
	output[ 3 ].TexCoord = float2( 1.f,  1.f );
	output[ 3 ].Color = float4( 0.f, 0.f, 0.f, 0.f );

	TriStream.Append( output[ 0 ] );
	TriStream.Append( output[ 1 ] );
	TriStream.Append( output[ 2 ] );
	TriStream.Append( output[ 3 ] );

	TriStream.RestartStrip();
}

technique11 text
{
	pass main
	{
		SetVertexShader( CompileShader( vs_4_0, vs_text() ) );
		SetGeometryShader( CompileShader( gs_4_0, gs_text() ) );
		SetPixelShader( CompileShader( ps_4_0, ps_flat() ) );
	}
}

#include "sky_box.hlsl"
#include "ground.hlsl"

// ----------------------------------------
// for Billboard
// ----------------------------------------
technique11 billboard
{
	pass main
	{
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_flat() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_flat() ) );
		// SetPixelShader( CompileShader( ps_4_0, ps_flat_with_flicker() ) );

		RASTERIZERSTATE = Default;
	}
}

#include "debug_axis.hlsl"
#include "debug_bullet.hlsl"

// ----------------------------------------
// for Shadow Map
// ----------------------------------------
float4 ps_shadow_map_debug( PS_INPUT input ) : SV_Target
{
	// float sz = shadow_texture.Sample( texture_sampler, input.TexCoord );
	float sz = model_texture.Sample( texture_sampler, input.TexCoord ).x;

	// sz *= 1000.f;
	// sz -= 999.5f;

	return float4( sz, sz, sz, 0.5f );
}

technique11 shadow_map
{
	pass main
	{
		SetBlendState( NoBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_shadow_map() ) );
		SetGeometryShader( NULL );
		SetPixelShader( NULL );

		RASTERIZERSTATE = Shadow;
	}
}

technique11 shadow_map_skin
{
	pass main
	{
		SetBlendState( NoBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_shadow_map_skin() ) );
		SetGeometryShader( NULL );
		SetPixelShader( NULL );

		RASTERIZERSTATE = Shadow;
	}
}

technique11 main_with_shadow
{
	pass main
	{
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_with_shadow() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_with_shadow() ) );

		RASTERIZERSTATE = Default;
	}
}

#include "2d.hlsl"

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