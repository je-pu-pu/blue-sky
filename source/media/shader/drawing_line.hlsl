GS_LINE_INPUT vs_drawing_line( VS_LINE_INPUT input, uint vertex_id : SV_VertexID )
{
	GS_LINE_INPUT output;

	output.Position = mul( input.Position, World );
    output.Position = mul( output.Position, View );
    output.Position = mul( output.Position, Projection );

	output.Color = input.Color + ObjectColor;

	// 色を変動させる
	static const float color_random_range = 0.1f;

	/// @todo 描画毎に vertex_id が変動する場合があるため色がちらつく問題に対応する
	if ( false )
	{
		output.Color.r += ( ( ( uint( Time *  5 ) + vertex_id ) % 8 ) / 4.f - 1.f ) * color_random_range;
		output.Color.g += ( ( ( uint( Time * 15 ) + vertex_id ) % 8 ) / 4.f - 1.f ) * color_random_range;
		output.Color.b += ( ( ( uint( Time * 25 ) + vertex_id ) % 8 ) / 4.f - 1.f ) * color_random_range;
		// output.Color.a -= ( ( uint( Time * 5 ) + vertex_id ) % 8 ) / 8.f * 0.5f;
	}

	// ぶらす
	if ( false )
	{
		float a = Time; // ( vertex_id ) / 10.f + Time * 10.f;
		output.Position.x += cos( vertex_id + a ) * 0.1f;
		output.Position.y += sin( vertex_id + Time % 10 / 10 ) * 0.1f;
	}

	return output;
}

/**
 * 手書き風の線を生成する
 *
 * input  : 0----------1
 *
 * output : 0----------2
 *          |          |
 *          1----------3
 *
 */
[maxvertexcount(4)]
void gs_drawing_line( line GS_LINE_INPUT input[2], inout TriangleStream<PS_FLAT_INPUT> Stream, uint primitive_id : SV_PrimitiveID )
{
	static const uint input_vertex_count = 2;
	static const uint output_vertex_count = 4;

	const float screen_width = ScreenWidth;
	const float screen_height = ScreenHeight;
	const float screen_ratio = ( screen_height / screen_width );

	static const float z_offset = -0.00001f;
	
	[unroll]
	for ( uint n = 0; n < input_vertex_count; n++ )
	{
		input[ n ].Position.z += z_offset;
		input[ n ].Position.xyz /= input[ n ].Position.w;
	}

	static const float LineTextureSize = 1024.f;
	
	static const float DrawingAccentPower = 1.5f;
	static const float DrawingAccentScale = 10.f;
	
	struct line_config
	{
		float v_offset;
		float line_width;
		uint  pattern_count;
	};

	static const line_config line_configs[ 5 ]= 
	{
		{   0.f, 32.f, 3 },
		{ 128.f, 32.f, 3 },
		{ 256.f, 64.f, 4 },
		{ 512.f, 64.f, 4 },
		{ 768.f, 64.f, 4 },
	};

	static const uint line_config_index = LineType;

	const float line_width_pixels = line_configs[ line_config_index ].line_width;
	const float line_width_scale = 0.5f + pow( abs( DrawingAccent ), DrawingAccentPower ) * DrawingAccentScale;
	
	float line_width = line_width_pixels * line_width_scale / screen_height;

	const float line_v_width = line_width_pixels / LineTextureSize;
	const float line_v_offset = line_configs[ line_config_index ].v_offset / LineTextureSize;

	// 更新パターン数
	const int pattern_count = line_configs[ line_config_index ].pattern_count;

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

		float line_length = length( float2( lx, ly ) );
		float line_length_ratio = line_length / length( float2( 1.f, 1.f ) );

		line_width = min( line_width, line_length ); // 線の長さが短い場合に線の幅が太くならないようにする

		// 線の向きを 90 度回転させた角度
		const float line_width_angle = atan2( ly, lx ) + Pi / 2.f;

		float dx = cos( line_width_angle ) * line_width * screen_ratio;
		float dy = sin( line_width_angle ) * line_width;

		PS_FLAT_INPUT output[ 4 ];

		/**
		 * 1----------------------------------------3
		 * |                                        |
		 * 0----------------------------------------2
		 */
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
			Stream.Append( output[ x ] );
		}

		Stream.RestartStrip();
	}
}

float4 ps_drawing_line( PS_FLAT_INPUT input ) : SV_Target
{
	return ( line_texture.Sample( u_wrap_texture_sampler, input.TexCoord ) + input.Color ); // * float4( 1.f, 1.f, 1.f, 0.5f );
}

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
	// Filter = MIN_MAG_MIP_POINT;
	Filter = ANISOTROPIC;
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

	/*
	pass debug
	{
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( NoDepthTest, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_drawing_line_debug() ) );
		SetGeometryShader( CompileShader( gs_4_0, gs_drawing_line_debug() ) );
		SetPixelShader( CompileShader( ps_4_0, ps_drawing_line_debug() ) );

		RASTERIZERSTATE = Debug;
	}
	*/

	pass debug_line
	{
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( NoDepthTest, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_drawing_line_debug() ) );
		SetGeometryShader( CompileShader( gs_4_0, gs_drawing_line_debug_line() ) );
		SetPixelShader( CompileShader( ps_4_0, ps_drawing_line_debug_line() ) );

		RASTERIZERSTATE = Default;
	}
}