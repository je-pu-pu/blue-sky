BlendState CanvasPenBlend
{
	BlendEnable[ 0 ] = True;
	
	SrcBlend = SRC_ALPHA;
	DestBlend = INV_SRC_ALPHA;
	
	// SrcBlend = ONE;
	// DestBlend = ONE;

	AlphaToCoverageEnable = False;
};

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

void add_pen_point( inout TriangleStream<COMMON_POS_UV_COLOR> TriStream , in GS_CANVAS_INPUT input, uint primitive_id )
{
	const float screen_width = ScreenWidth;
	const float screen_height = ScreenHeight;
	const float screen_ratio = ( screen_height / screen_width );

	const float l = 0.2f * input.Pressure;
	const float hw = l * 0.5f * screen_ratio;
	const float hh = l * 0.5f;
	
	COMMON_POS_UV_COLOR output[ 4 ];
	
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
GS_CANVAS_INPUT vs_canvas( VS_CANVAS_INPUT input, uint vertex_id : SV_VertexID )
{
	GS_CANVAS_INPUT output;

	output.Position = common_wvp_pos( input.Position );
	output.Pressure = input.Pressure;
	output.Color = input.Color;

	// 色変動
	if ( true )
	{
		static const float color_random_range = 0.01f;
		output.Color.r += ( ( ( uint( Time * 5 ) + vertex_id ) % 8 ) / 4.f - 1.f ) * color_random_range;
		output.Color.g += ( ( ( uint( Time * 15 ) + vertex_id ) % 8 ) / 4.f - 1.f ) * color_random_range;
		output.Color.b += ( ( ( uint( Time * 25 ) + vertex_id ) % 8 ) / 4.f - 1.f ) * color_random_range;
	}

	// サイズ変動
	if ( true )
	{
		static const float factor = 0.75f;
		output.Pressure += ( random( output.Position.x * 100.f + output.Position.y * 10.f + output.Position.z + Time ) - 0.5f ) * factor;
	}

	// 位置変動
	if ( false )
	{
		static const float factor = 0.05f;
		const float mx = ( ( vertex_id + 8  ) % 10 ) + 1;
		const float my = ( ( vertex_id + 10 ) % 28 ) + 1;
		const float mz = ( ( vertex_id + 15 ) % 15 ) + 1;

		output.Position.x += cos( vertex_id + Time / mx ) * factor;
		output.Position.y += sin( vertex_id + Time / my ) * factor;
		output.Position.z += sin( vertex_id + Time / mz ) * factor;
	}

	return output;
}

/**
 * Canvas
 *
 */
[maxvertexcount(6)]
void gs_canvas( point GS_CANVAS_INPUT input[ 1 ], inout TriangleStream<COMMON_POS_UV_COLOR> TriStream, uint primitive_id : SV_PrimitiveID )
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
PS_CANVAS_OUTPUT ps_canvas( COMMON_POS_UV_COLOR input )
{
	PS_CANVAS_OUTPUT output;
	
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

technique11 drawing_point
{
	pass main
    {
		SetBlendState( CanvasPenBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		// SetDepthStencilState( NoWriteDepth, 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

        SetVertexShader( CompileShader( vs_4_0, vs_canvas() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( CompileShader( gs_4_0, gs_canvas() ) );
        SetPixelShader( CompileShader( ps_4_0, ps_canvas() ) );

		RASTERIZERSTATE = Default;
    }
}
