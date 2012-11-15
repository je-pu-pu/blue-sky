static const int ShadowMapCascadeLevels = 3;
static const float Pi = 3.14159265f;

Texture2D model_texture : register( t0 );
Texture2D line_texture : register( t0 );
Texture2D shadow_texture : register( t1 );

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

struct VS_INPUT
{
	float4 Position : SV_POSITION;
	float3 Normal   : NORMAL0;
	float2 TexCoord : TEXCOORD0;
};

struct GS_INPUT
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
};

struct VS_LINE_INPUT
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR0;
};

struct VS_LINE_OUTPUT
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR0;
	float Depth : TEXCOORD0; /// ???
};

struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
	float4 Color    : COLOR0;
};

struct PS_SHADOW_INPUT
{
	float4 Position : SV_POSITION;
	float4 ShadowTexCoord[ ShadowMapCascadeLevels ] : TEXCOORD;
	float2 TexCoord : TEXCOORD4;
	float Depth : TEXCOORD5;
};

GS_INPUT vs( VS_INPUT input, uint vertex_id : SV_VertexID )
{
	GS_INPUT output;

	output.Position = mul( input.Position, World );
    output.Position = mul( output.Position, View );
    output.Position = mul( output.Position, Projection );
	output.TexCoord = input.TexCoord;

	// ぶらす
	// float a = ( vertex_id ) / 10.f + Time * 10.f;
	// output.Position.x += cos( a ) * 0.01f;
	// output.Position.y += sin( Time ) * 0.01f;

	// output.Position = input.Position;

    return output;
}

PS_INPUT vs_to_ps( VS_INPUT input, uint vertex_id : SV_VertexID )
{
	PS_INPUT output;

	output.Position = mul( input.Position, World );
    output.Position = mul( output.Position, View );
    output.Position = mul( output.Position, Projection );
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

VS_LINE_OUTPUT vs_line( VS_LINE_INPUT input, uint vertex_id : SV_VertexID )
{
	VS_LINE_OUTPUT output;

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

[maxvertexcount(12)]
void gs_pass( triangle GS_INPUT input[3], inout TriangleStream<PS_INPUT> TriStream )
{
	for ( uint n = 0; n < 3; n++ )
	{
		PS_INPUT output;
		output.Position = input[ n ].Position;
		output.TexCoord = input[ n ].TexCoord;
		output.Color = float4( 1.f, 1.f, 1.f, 1.f );

		TriStream.Append( output );
	}

	TriStream.RestartStrip();
}

[maxvertexcount(4)]
void gs_line( line VS_LINE_OUTPUT input[2], inout TriangleStream<PS_INPUT> TriStream, uint primitive_id : SV_PrimitiveID )
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
	
	static const float line_v_width = 32.f / 1024.f;

	for ( uint n = 0; n < input_vertex_count; n++ )
	{
		input[ n ].Position.z += z_offset;
		input[ n ].Position.xyz /= input[ n ].Position.w;
	}

	const float line_width_scale = 0.5f;
	const float line_width = 32.f / screen_height * line_width_scale;

	{
		uint n = 0;
		uint m = 1;

		float ly = ( input[ m ].Position.y * screen_ratio ) - ( input[ n ].Position.y * screen_ratio );
		float lx = input[ m ].Position.x - input[ n ].Position.x;
		
		int redraw_seed = uint( Time * 5.f ) + primitive_id;

		// float line_index = ( uint( Time + primitive_id % 10 / 10.f ) + primitive_id ) % 3; // 全ての線がばらばらのタイミングで更新される
		float line_index = redraw_seed % 3; // 全ての線が統一されたタイミングで更新される
		float line_v_origin = ( line_index * line_v_width );

		float line_u_origin = ( redraw_seed ) * 0.1f;
		float line_length_ratio = length( float2( lx, ly ) ) / length( float2( 2.f, 2.f ) );

		float angle = atan2( ly, lx );
		angle += Pi / 2.f;

		float dx = cos( angle ) * line_width * screen_ratio;
		float dy = sin( angle ) * line_width;

		PS_INPUT output[ 4 ];

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

float4 ps( PS_INPUT input ) : SV_Target
{
	return model_texture.Sample( texture_sampler, input.TexCoord ) + input.Color;
}

float4 ps_wrap( PS_INPUT input ) : SV_Target
{
	return model_texture.Sample( wrap_texture_sampler, input.TexCoord ) + input.Color;
}

float4 ps_line( noperspective PS_INPUT input ) : SV_Target
{
	return line_texture.Sample( u_wrap_texture_sampler, input.TexCoord ) + input.Color;
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

RasterizerState Default
{
	CullMode = BACK;
};

RasterizerState WireFrame
{
	FILLMODE = WIREFRAME;
};

RasterizerState Shadow
{
	CullMode = NONE;
	// SlopeScaledDepthBias = 1.f;
};

// ----------------------------------------
// main
// ----------------------------------------
technique11 main
{
	pass pass1
    {
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

        SetVertexShader( CompileShader( vs_4_0, vs_to_ps() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, ps_wrap() ) );

		RASTERIZERSTATE = Default;
    }
}

technique11 drawing_line
{
	pass pass_line
	{
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( NoWriteDepth, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_line() ) );
		SetGeometryShader( CompileShader( gs_4_0, gs_line() ) );
		SetPixelShader( CompileShader( ps_4_0, ps_line() ) );

		// RASTERIZERSTATE = WireFrame;
	}
}

// ----------------------------------------
// for Text
// ----------------------------------------
GS_INPUT vs_text( VS_INPUT input )
{
	GS_INPUT output;
	output.Position = input.Position;
	output.TexCoord = input.TexCoord;

	return output;
}

[maxvertexcount(4)]
void gs_text( triangle GS_INPUT input[3], inout TriangleStream<PS_INPUT> TriStream )
{
	PS_INPUT output[ 4 ];

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
		SetPixelShader( CompileShader( ps_4_0, ps() ) );
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

		SetVertexShader( CompileShader( vs_4_0, vs_to_ps() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps() ) );

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
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_to_ps() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps() ) );

		RASTERIZERSTATE = Default;
	}
}

// ----------------------------------------
// for Bullet debug
// ----------------------------------------
struct BULLET_DEBUG_INPUT
{
	float4 Position : SV_POSITION;
	float3 Color    : COLOR0;
};

BULLET_DEBUG_INPUT vs_bullet_debug( BULLET_DEBUG_INPUT input )
{
	BULLET_DEBUG_INPUT output;

	output.Position = mul( input.Position, View );
    output.Position = mul( output.Position, Projection );
	output.Color = input.Color;

	return output;
}

float4 ps_bullet_debug( BULLET_DEBUG_INPUT input ) : SV_Target
{
	return float4( input.Color.x, input.Color.y, input.Color.z, 0.5f );
}

technique11 bullet
{
	pass main
	{
		SetVertexShader( CompileShader( vs_4_0, vs_bullet_debug() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_bullet_debug() ) );
	}
}

// ----------------------------------------
// for Shadow Map
// ----------------------------------------
cbuffer ShadowMapBuffer : register( b10 )
{
	matrix ShadowViewProjection[ ShadowMapCascadeLevels ];
	float4 ShadowMapViewDepthPerCascadeLevel;
};

static const float4 cascade_colors[ 4 ] = 
{
    float4 ( 1.2f, 1.0f, 1.0f, 1.0f ),
    float4 ( 1.0f, 1.2f, 1.0f, 1.0f ),
    float4 ( 1.0f, 1.0f, 1.2f, 1.0f ),
    float4 ( 1.2f, 1.2f, 0.0f, 1.0f ),
};

PS_SHADOW_INPUT vs_with_shadow( VS_INPUT input )
{
	PS_SHADOW_INPUT output;

	output.Position = mul( input.Position, World );
    output.Position = mul( output.Position, View );
    output.Position = mul( output.Position, Projection );
	output.TexCoord = input.TexCoord;
	
	for ( int n = 0; n < ShadowMapCascadeLevels; n++ )
	{
		output.ShadowTexCoord[ n ] = mul( input.Position, World );
		output.ShadowTexCoord[ n ] = mul( output.ShadowTexCoord[ n ], ShadowViewProjection[ n ] );
		
		output.ShadowTexCoord[ n ] /= output.ShadowTexCoord[ n ].w;
	
		output.ShadowTexCoord[ n ].x = ( output.ShadowTexCoord[ n ].x + 1.f ) / 2.f;
		output.ShadowTexCoord[ n ].y = ( -output.ShadowTexCoord[ n ].y + 1.f ) / 2.f;
		output.ShadowTexCoord[ n ].z -= 0.00001f;
		// output.ShadowTexCoord[ n ].z -= 0.000005f;
	}

	output.Depth = mul( mul( input.Position, World ), View ).z;

	return output;
}

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

	if ( shadow_tex_coord.z >= sz )
	{
		const float4 shadow_color = float4( 0.5f, 0.5f, 0.75f, 1.f );

		const float a = 0.75f;

		/*
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
	}

	return model_texture.Sample( wrap_texture_sampler, input.TexCoord ) * shadow; // * cascade_colors[ cascade_index ];
}

PS_INPUT vs_shadow_map( VS_INPUT input )
{
	PS_INPUT output;
	
	output.Position = mul( input.Position, World );
    output.Position = mul( output.Position, ShadowViewProjection[ 0 ] );
	output.TexCoord = input.TexCoord;
	output.Color = float4( 0.f, 0.f, 0.f, 0.f );

	return output;
}

float4 ps_shadow_map_debug( PS_INPUT input ) : SV_Target
{
	// float sz = shadow_texture.Sample( texture_sampler, input.TexCoord );
	float sz = model_texture.Sample( texture_sampler, input.TexCoord ).x;

	sz *= 1000.f;
	sz -= 999.5f;

	return float4( sz, sz, sz, 0.5f );
}

technique11 shadow_map
{
	pass main
	{
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_shadow_map() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps() ) );

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

PS_INPUT vs_sprite( PS_INPUT input )
{
	PS_INPUT output = input;

	output.Position = mul( input.Position, Transform );

	return output;
}

float4 ps_sprite( PS_INPUT input ) : SV_Target
{
	return model_texture.Sample( texture_sampler, input.TexCoord ) * input.Color;
}

float4 ps_sprite_add( PS_INPUT input ) : SV_Target
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