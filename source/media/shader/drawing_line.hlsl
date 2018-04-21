/**
 * デバッグ用ラスタライザ
 *
 */
RasterizerState Debug
{
	// CullMode = Front;
	// CullMode = BACK;
	CullMode = NONE; // 背面カリングを行わない
	MultisampleEnable = True;
};

/**
 * 手書き風の線のためのバーテックスシェーダー
 *
 */
GS_LINE_INPUT vs_drawing_line_debug( VS_LINE_INPUT input )
{
	GS_LINE_INPUT output;

	output.Position = input.Position;
	output.Position = mul( input.Position, World );
    output.Position = mul( output.Position, View );
	output.Position = mul( output.Position, Projection );

	/*
	// output.Position.z = max( output.Position.z, 0.5f );
	*/

	output.Color = input.Color;

	return output;
}

/**
 * 手書き風の線を生成するジオメトリシェーダーから利用する共通関数
 *
 * input  : 0----------1
 *
 * output : 1----------3
 *          |          |
 *          0----------2
 *
 */
void get_drawing_line_common( line GS_LINE_INPUT input[ 2 ], out PS_FLAT_INPUT output[ 4 ] )
{
	static const uint input_vertex_count = 2;
	static const uint output_vertex_count = 4;
	
	const float screen_ratio = ( ScreenHeight / ScreenWidth );

	{
		[unroll]
		for ( uint n = 0; n < input_vertex_count; n++ )
		{
			// input[ n ].Position /= input[ n ].Position.w;
			input[ n ].Position.xyz /= input[ n ].Position.w;

			if ( input[ n ].Position.z < 0 || input[ n ].Position.z > 1 )
			{
				input[ n ].Position.z = -0.0000001;
				input[ n ].Position.w = 0.05f;
			}
		}
	}

	{
		static const uint n = 0;
		static const uint m = 1;

		// input[ n ].Position.z = input[ m ].Position.z;
		// input[ m ].Position.z = input[ n ].Position.z;

		const float lx = input[ m ].Position.x - input[ n ].Position.x;
		const float ly = ( input[ m ].Position.y * screen_ratio ) - ( input[ n ].Position.y * screen_ratio );

		// 線の向きを 90 度回転させて
		const float line_width_angle = atan2( ly, lx ) + Pi / 2.f;

		// 線のサイズ
		const float line_width_half = 0.1f;
		const float edge_scale_n = 1.f; // abs( input[ n ].Position.z ) / abs( input[ m ].Position.z );
		const float edge_scale_m = 1.f; // abs( input[ m ].Position.z ) / abs( input[ n ].Position.z );

		// 方向
		float4 vw = float4( cos( line_width_angle ), sin( line_width_angle ), 0.f, 0.f ) * line_width_half;

		static const float v_bottom = 1.f; // 32.f / 1024.f;

		output[ 0 ].Position = input[ n ].Position - vw * edge_scale_n;
		output[ 0 ].TexCoord = float2( 0.f, 0.f );
		output[ 0 ].Color = input[ n ].Color;

		output[ 1 ].Position = input[ n ].Position + vw * edge_scale_n;
		output[ 1 ].TexCoord = float2( 0.f, v_bottom );
		output[ 1 ].Color = input[ n ].Color;

		output[ 2 ].Position = input[ m ].Position - vw * edge_scale_m;
		output[ 2 ].TexCoord = float2( 1.f, 0.f );
		output[ 2 ].Color = input[ m ].Color;

		output[ 3 ].Position = input[ m ].Position + vw * edge_scale_m;
		output[ 3 ].TexCoord = float2( 1.f, v_bottom );
		output[ 3 ].Color = input[ m ].Color;
	}

	{
		for ( uint n = 0; n < output_vertex_count; n++ )
		{
			// output[ n ].Position = mul( output[ n ].Position, World );
			// output[ n ].Position = mul( output[ n ].Position, View );
			// output[ n ].Position = mul( output[ n ].Position, Projection );

			// output[ n ].Position.z = clamp( output[ n ].Position.z, -1.f, 1.f );
			output[ n ].Position.xyz *= output[ n ].Position.w;
			
			// output[ n ].Position.w = 1.f;
		}
	}
}

/**
 * 手書き風の線を生成するジオメトリシェーダー
 *
 */
[maxvertexcount(4)]
void gs_drawing_line_debug( line GS_LINE_INPUT input[2], inout TriangleStream<PS_FLAT_INPUT> Stream )
{
	PS_FLAT_INPUT output[ 4 ];

	get_drawing_line_common( input, output );

	Stream.Append( output[ 0 ] );
	Stream.Append( output[ 1 ] );
	Stream.Append( output[ 2 ] );
	Stream.Append( output[ 3 ] );

	Stream.RestartStrip();
}

/**
 * 手書き風の線を生成するジオメトリシェーダー ( 輪郭線 )
 *
 */
[maxvertexcount(7)]
void gs_drawing_line_debug_line( line GS_LINE_INPUT input[ 2 ], inout LineStream<PS_FLAT_INPUT> Stream )
{
	PS_FLAT_INPUT output[ 4 ];

	get_drawing_line_common( input, output );

	Stream.Append( output[ 0 ] );
	Stream.Append( output[ 1 ] );
	Stream.Append( output[ 3 ] );
	Stream.Append( output[ 2 ] );
	Stream.Append( output[ 0 ] );
	Stream.RestartStrip();

	Stream.Append( output[ 1 ] );
	Stream.Append( output[ 2 ] );
	Stream.RestartStrip();
}

SamplerState drawing_line_texture_sampler
{
	Filter = MIN_MAG_MIP_POINT;
    AddressU = WRAP;
    AddressV = WRAP;
    AddressW = WRAP;
	BorderColor = float4( 1.f, 1.f, 1.f, 1.f );
	// ComparisonFunc = ALWAYS;
};

/**
 * 手書き風線 デバッグ用ピクセルシェーダー
 *
 */
float4 ps_drawing_line_debug( noperspective PS_FLAT_INPUT input ) : SV_Target
// float4 ps_drawing_line_debug( PS_FLAT_INPUT input ) : SV_Target
{
	return float4( 0.f, 0.f, 1.f, 1.f );
	// return ( line_texture.Sample( drawing_line_texture_sampler, input.TexCoord ) );
}

/**
 * 手書き風線 デバッグ用ピクセルシェーダー ( 輪郭線 )
 *
 */
float4 ps_drawing_line_debug_line( PS_FLAT_INPUT input ) : SV_Target
{
	return float4( 0.f, 0.f, 0.f, 0.f );
	return float4( 1.f, 0.f, 0.f, 0.75f );
}