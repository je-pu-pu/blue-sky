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
	float  Depth    : DEPTH; // ランダムに移動する前の Position.z
};

struct PS_CANVAS_INPUT
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
	float4 Color    : COLOR0;
	float  Depth    : DEPTH;
};

void add_pen_point( inout TriangleStream<PS_CANVAS_INPUT> TriStream , in GS_CANVAS_INPUT input, uint primitive_id )
{
	const float screen_width = ScreenWidth;
	const float screen_height = ScreenHeight;
	const float screen_ratio = ( screen_height / screen_width );

	const float l = 0.2f * input.Pressure;
	const float hw = l * 0.5f * screen_ratio;
	const float hh = l * 0.5f;
	
	PS_CANVAS_INPUT output[ 4 ];
	
	// left top
	output[ 0 ].Position = input.Position + float4( -hw, -hh, 0.f, 0.f );
	output[ 0 ].TexCoord.xy = float2( 0.f, 1.f );
	output[ 0 ].Depth = input.Depth;


	// right top
	output[ 1 ].Position = input.Position + float4( +hw, -hh, 0.f, 0.f );
	output[ 1 ].TexCoord.xy = float2( 1.f, 1.f );
	output[ 1 ].Depth = input.Depth;

	// left bottom
	output[ 2 ].Position = input.Position + float4( -hw, +hh, 0.f, 0.f );
	output[ 2 ].TexCoord.xy = float2( 0.f, 0.f );
	output[ 2 ].Depth = input.Depth;

	// right bottom
	output[ 3 ].Position = input.Position + float4( +hw, +hh, 0.f, 0.f );
	output[ 3 ].TexCoord.xy = float2( 1.f, 0.f );
	output[ 3 ].Depth = input.Depth;

	for ( uint y = 0; y < 4; ++y )
	{
		output[ y ].Color = input.Color;
	}

	TriStream.Append( output[ 0 ] );
	TriStream.Append( output[ 2 ] );
	TriStream.Append( output[ 1 ] );
	TriStream.Append( output[ 3 ] );
	// TriStream.RestartStrip();
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
	output.Depth = output.Position.z;

	// 色変動
	if ( true )
	{
		static const float color_random_range = 0.05f;
		output.Color.r += ( ( ( uint( Time * 5 ) + vertex_id ) % 8 ) / 4.f - 1.f ) * color_random_range;
		output.Color.g += ( ( ( uint( Time * 15 ) + vertex_id ) % 8 ) / 4.f - 1.f ) * color_random_range;
		output.Color.b += ( ( ( uint( Time * 25 ) + vertex_id ) % 8 ) / 4.f - 1.f ) * color_random_range;
	}

	// サイズ変動
	if ( false )
	{
		static const float factor = 0.1f;
		output.Pressure += ( random( ( output.Position.x * 100.f + output.Position.y * 10.f + output.Position.z ) * Time * vertex_id * 100.f ) - 0.5f ) * factor;
	}

	// 位置変動
	if ( false )
	{
		static const float factor = 0.01f;
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
[maxvertexcount(4)]
void gs_canvas( point GS_CANVAS_INPUT input[ 1 ], inout TriangleStream<PS_CANVAS_INPUT> TriStream, uint primitive_id : SV_PrimitiveID )
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
PS_CANVAS_OUTPUT ps_canvas( PS_CANVAS_INPUT input )
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

	// 中心からの距離に応じて Depth を設定
	if ( false )
	{
		const float2 d = input.TexCoord - float2( 0.5f, 0.5f );
		const float l = sqrt( d.x * d.x + d.y * d.y ); // 円の中心からの距離 ( 0.f .. 1.f )

		output.Depth = sin( l * ( PI / 2.f ) ); // 0.f .. 1.f をそのまま Depth に設定
	}
	else
	{
		// 通常のポイントスプライトの Depth 設定
		output.Depth = input.Depth / input.Position.w;
	}
	

	// output.Depth = input.Position.z;

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
