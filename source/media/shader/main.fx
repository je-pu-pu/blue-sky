static const int ShadowMapCascadeLevels = 3;
static const float Pi = 3.14159265f;
static const uint MaxBones = 200;

Texture2D model_texture : register( t0 );		/// モデルのテクスチャ
Texture2D line_texture : register( t0 );		/// 線のテクスチャ
Texture2D shadow_texture : register( t1 );		/// シャドウマップ
Texture2D paper_texture : register( t2 );		/// 紙の質感テクスチャ

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

cbuffer GameConstantBuffer : register( b0 )
{
	matrix Projection;
	float ScreenWidth;
	float ScreenHeight;
};

cbuffer FrameConstantBuffer : register( b1 )
{
	matrix View;
	float Time;
	uint TimeBeat;
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
	float DrawingAccent;
	float3 dummy;
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
	float Depth     : TEXCOORD1; /// ???
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

PS_INPUT vs_main( VS_INPUT input, uint vertex_id : SV_VertexID )
{
	PS_INPUT output;

	output.Position = mul( input.Position, World );
    output.Position = mul( output.Position, View );
    output.Position = mul( output.Position, Projection );
	
	output.Normal = mul( input.Normal, ( float3x3 ) World );
	
	output.TexCoord = input.TexCoord;
	output.Color = float4( 0.f, 0.f, 0.f, 0.f );

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
	output.Color = float4( 0.f, 0.f, 0.f, 1.f );

	return output;
}

PS_FLAT_INPUT vs_flat( VS_INPUT input, uint vertex_id : SV_VertexID )
{
	PS_FLAT_INPUT output;

	output.Position = mul( input.Position, World );
    output.Position = mul( output.Position, View );
    output.Position = mul( output.Position, Projection );
	output.TexCoord = input.TexCoord;
	output.Color = float4( 0.f, 0.f, 0.f, 0.f );

	return output;
}

GS_LINE_INPUT vs_drawing_line( VS_LINE_INPUT input, uint vertex_id : SV_VertexID )
{
	GS_LINE_INPUT output;

	output.Position = mul( input.Position, World );
    output.Position = mul( output.Position, View );
    output.Position = mul( output.Position, Projection );

	output.Color = input.Color + ObjectColor;
	
	output.Depth = mul( mul( input.Position, World ), View ).z;

	// 色を変動させる
	static const float color_random_range = 0.1f;

	output.Color.r += ( ( ( uint( Time * 5 ) + vertex_id ) % 8 ) / 4.f - 1.f ) * color_random_range;
	output.Color.g += ( ( ( uint( Time * 15 ) + vertex_id ) % 8 ) / 4.f - 1.f ) * color_random_range;
	output.Color.b += ( ( ( uint( Time * 25 ) + vertex_id ) % 8 ) / 4.f - 1.f ) * color_random_range;
	// output.Color.a -= ( ( uint( Time * 5 ) + vertex_id ) % 8 ) / 8.f * 0.5f;

	// ぶらす
	// float a = Time; // ( vertex_id ) / 10.f + Time * 10.f;
	// output.Position.x += cos( vertex_id + a ) * 0.001f;
	// output.Position.y += ( vertex_id + Time % 10 / 10 ) * 0.04f;

	// output.Position = input.Position;

	return output;
}

[maxvertexcount(4)]
void gs_drawing_line( line GS_LINE_INPUT input[2], inout TriangleStream<PS_FLAT_INPUT> TriStream, uint primitive_id : SV_PrimitiveID )
{
	static const uint input_vertex_count = 2;
	static const uint output_vertex_count = 4;

	const float screen_width = ScreenWidth;
	const float screen_height = ScreenHeight;
	const float screen_ratio = ( screen_height / screen_width );

	static const float w_fix = 1.f;
	static const float z_fix = 0.f;

	// static const float z_offset = 0.f;
	static const float z_offset = -0.00001f;
	
	[unroll]
	for ( uint n = 0; n < input_vertex_count; n++ )
	{
		input[ n ].Position.z += z_offset;
		input[ n ].Position.xyz /= input[ n ].Position.w;
	}

	static const float LineTextureSize = 1024.f;
	
	static const float DrawingAccentPower = 2.f;
	static const float DrawingAccentScale = 10.f;

	const float line_width_pixels = 32.f;
	const float line_width_scale = 0.5f + pow( abs( DrawingAccent ), DrawingAccentPower ) * DrawingAccentScale;
	const float line_width = line_width_pixels * line_width_scale / screen_height;

	const float line_v_width = line_width_pixels / LineTextureSize;
	const float line_v_offset = ( 0.f ) / LineTextureSize;

	const int pattern_count = 3;	// 更新パターン

	{
		uint n = 0;
		uint m = 1;

		float ly = ( input[ m ].Position.y * screen_ratio ) - ( input[ n ].Position.y * screen_ratio );
		float lx = input[ m ].Position.x - input[ n ].Position.x;
		
		uint redraw_seed = uint( Time * 5.f ) + primitive_id;

		float line_index = ( uint( Time + primitive_id % 10 / 10.f ) + primitive_id ) % pattern_count; // 全ての線がばらばらのタイミングで更新される
		// float line_index = redraw_seed % pattern_count; // 全ての線が統一されたタイミングで更新される
		float line_v_origin = line_v_offset + ( line_index * line_v_width );

		float line_u_origin = ( redraw_seed ) * 0.1f;
		/// float line_length_ratio = 1.f; // 
		float line_length_ratio = length( float2( lx, ly ) ) / length( float2( 1.f, 1.f ) );

		float angle = atan2( ly, lx );
		angle += Pi / 2.f;

		float dx = cos( angle ) * line_width * screen_ratio;
		float dy = sin( angle ) * line_width;

		PS_FLAT_INPUT output[ 4 ];

		output[ 0 ].Position = input[ n ].Position + float4( -dx, -dy, 0.f, 0.f );
		output[ 0 ].TexCoord = float2( line_u_origin, line_v_origin );
		output[ 0 ].Color = input[ 0 ].Color;

		output[ 1 ].Position = input[ n ].Position + float4( +dx, +dy, 0.f, 0.f );
		output[ 1 ].TexCoord = float2( line_u_origin, line_v_origin + line_v_width );
		output[ 1 ].Color = input[ 0 ].Color;

		output[ 2 ].Position = input[ m ].Position + float4( -dx, -dy, 0.f, 0.f );
		output[ 2 ].TexCoord = float2( line_u_origin + line_length_ratio, line_v_origin );
		output[ 2 ].Color = input[ 1 ].Color;

		output[ 3 ].Position = input[ m ].Position + float4( +dx, +dy, 0.f, 0.f );
		output[ 3 ].TexCoord = float2( line_u_origin + line_length_ratio, line_v_origin + line_v_width );
		output[ 3 ].Color = input[ 1 ].Color;

		for ( uint x = 0; x < output_vertex_count; ++x )
		{
			output[ x ].Position.xyz *= output[ x ].Position.w;

			TriStream.Append( output[ x ] );
		}
		
		TriStream.RestartStrip();
	}
}

float4 ps_drawing_line( noperspective PS_FLAT_INPUT input ) : SV_Target
{
	return ( line_texture.Sample( u_wrap_texture_sampler, input.TexCoord ) + input.Color ); // * float4( 1.f, 1.f, 1.f, 0.5f );
}

float4 ps_flat( PS_FLAT_INPUT input ) : SV_Target
{
	return model_texture.Sample( texture_sampler, input.TexCoord ) + input.Color;
}

float4 ps_flat_with_flicker( PS_FLAT_INPUT input ) : SV_Target
{
	return model_texture.Sample( texture_sampler, input.TexCoord + float2( sin( input.TexCoord.y * 50.f + TimeBeat ) * 0.01f, 0.f ) ) + input.Color;
}

float4 ps_main_wrap( PS_INPUT input ) : SV_Target
{
	// return float4( input.Normal * 0.5f + float3( 0.5f, 0.5f, 0.5f ), 1.f );

	return model_texture.Sample( wrap_texture_sampler, input.TexCoord ); // /* + input.Color */ * float4( input.Normal, 1.f );
}

float4 ps_debug( PS_INPUT input ) : SV_Target
{
	return float4( 1.f, 0.f, 0.f, 0.25f );
}

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
	// DepthEnable = False;
	DepthWriteMask = ZERO;
};

DepthStencilState WriteDepth
{
	// DepthEnable = False;
	DepthWriteMask = ALL;
};

DepthStencilState NoDepthTest
{
	DepthEnable = False;
};

RasterizerState Default
{
	CullMode = BACK;
	// CullMode = NONE;
	MultisampleEnable = True;
};

RasterizerState WireFrame
{
	FILLMODE = WIREFRAME;
};

RasterizerState Shadow
{
	CullMode = NONE;
	SlopeScaledDepthBias = 1.f;
};

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
	
	// output.Normal = input.Normal;
	output.Normal = normalize( mul( input.Normal, ( float3x3 ) transpose( World ) ) );
	
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
	
	// output.Normal = input.Normal;
	output.Normal = normalize( mul( input.Normal, ( float3x3 ) transpose( World ) ) );
	
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


	// ---


	float4 shadow = float4( 1.f, 1.f, 1.f, 1.f );
	float sz = ( float ) shadow_texture.Sample( shadow_texture_sampler, shadow_tex_coord.xy );

	// if ( input.Position.z >= sz )
	if ( shadow_tex_coord.z >= sz )
	{
		// 影

		const float4 shadow_color = float4( 0.75f, 0.75f, 1.f, 1.f );

		const float a = 0.75f;

		/*
		// 端に行くほど影を透明にする？
		float a = abs( shadow_tex_coord.x - 0.5f ) + abs( shadow_tex_coord.y - 0.5f ) / 0.5f;

		a = min( a, 1.f );
		a = 1.f - pow( a, 3 );

		if ( shadow_tex_coord.z < ( float ) shadow_texture.Sample( shadow_texture_sampler, shadow_tex_coord.xy + float2( 1.f / 1024.f, 1.f ) ) )
		{
			a /= 2.f;
		}
		*/
		
		shadow = float4( 1.f, 1.f, 1.f, 1.f ) * ( 1.f - a ) + shadow_color * a;
		shadow.a = 1.f;

		// 紙の質感を追加する
		if ( true )
		{
			const float Scale = 0.5f; // 0.75f + DrawingAccent * 0.25f;
			
			const float PaperTextureWidth  = 512.f * Scale;
			const float PaperTextureHeight = 512.f * Scale;

			float2 uv = float2(
				input.Position.x / ScreenWidth  * ( ScreenWidth  / PaperTextureWidth  ),
				input.Position.y / ScreenHeight * ( ScreenHeight / PaperTextureHeight )
			);

			// アクセントを加える
			uv += float2( TimeBeat * 0.2f, 0.f );
			
			float4 paper = paper_texture.Sample( wrap_texture_sampler, uv );
			shadow.rgb = float3( 1.f, 1.f, 1.f ) * ( 1.f - paper.a ) + shadow.rgb * paper.a;
						
			/*
			shadow.r = input.Position.x / ScreenWidth;
			shadow.g = input.Position.y / ScreenHeight;
			shadow.b = 0.f;
			
			*/

			// return shadow;
		}
	}

	// return float4( input.Normal, 1.f );

	// return float4( sz, sz, sz, 1.f );
	// return float4( input.Position.z, input.Position.z, input.Position.z, 1.f );

	// return paper_texture.Sample( wrap_texture_sampler, input.TexCoord );

	static const float4 cascade_colors[ 4 ] = 
	{
		float4 ( 1.2f, 1.0f, 1.0f, 1.0f ),
		float4 ( 1.0f, 1.2f, 1.0f, 1.0f ),
		float4 ( 1.0f, 1.0f, 1.2f, 1.0f ),
		float4 ( 1.2f, 1.2f, 0.0f, 1.0f ),
	};

	return model_texture.Sample( wrap_texture_sampler, input.TexCoord ) * shadow; // * cascade_colors[ cascade_index ];
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
// for drawing line
// ----------------------------------------
technique11 drawing_line
{
	pass main
	{
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( NoWriteDepth, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_drawing_line() ) );
		SetGeometryShader( CompileShader( gs_4_0, gs_drawing_line() ) );
		SetPixelShader( CompileShader( ps_4_0, ps_drawing_line() ) );

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

// ----------------------------------------
// for SkyBox
// ----------------------------------------
technique11 sky_box
{
	pass main
	{
		SetDepthStencilState( NoWriteDepth, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_flat() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_flat() ) );

		RASTERIZERSTATE = Default;
	}
}

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

// ----------------------------------------
// for debug axis
// ----------------------------------------
VS_LINE_INPUT vs_line( VS_LINE_INPUT input )
{
	input.Position = mul( input.Position, World );
    input.Position = mul( input.Position, View );
    input.Position = mul( input.Position, Projection );

	return input;
}

float4 ps_line( VS_LINE_INPUT input ) : SV_Target
{
	return float4( input.Color.x, input.Color.y, input.Color.z, 0.5f );
}

technique11 simple_line
{
	pass main
	{
		SetDepthStencilState( NoDepthTest, 0xFFFFFFFF );
		
		SetVertexShader( CompileShader( vs_4_0, vs_line() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_line() ) );
	}
}

// ----------------------------------------
// for Bullet debug
// ----------------------------------------
VS_LINE_INPUT vs_bullet_debug( VS_LINE_INPUT input )
{
	VS_LINE_INPUT output;

	output.Position = mul( input.Position, View );
    output.Position = mul( output.Position, Projection );
	output.Color = input.Color;

	return output;
}

technique11 bullet
{
	pass main
	{
		SetVertexShader( CompileShader( vs_4_0, vs_bullet_debug() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_line() ) );
	}
}

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

// ----------------------------------------
// for 2D ( Fader, debug ウィンドウで使用 )
// ----------------------------------------
PS_INPUT vs_2d( VS_INPUT input )
{
	PS_INPUT output = ( PS_INPUT ) 0;

	output.Position = input.Position;
	output.TexCoord = input.TexCoord;
	// output.Color = ObjectColor;

	return output;
}

float4 ps_2d( PS_INPUT input ) : SV_Target
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
		SetDepthStencilState( NoWriteDepth, 0xFFFFFFFF );

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