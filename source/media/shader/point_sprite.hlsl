// ----------------------------------------
// for debug axis
// ----------------------------------------
COMMON_POS_COLOR vs_point_sprite( COMMON_POS_NORM input )
{
	COMMON_POS_COLOR output;
	output.Position = common_wv_pos( input.Position );

	const float f = clamp( input.Position.y * 0.1f, 0.f, 1.f );
	output.Color = lerp( float4( 1.f, 0.f, 0.f, 0.5f ), float4( 1.f, 1.f, 0.f, 0.5f ), float4( f, f, f, f ) );
	
	return output;
}

RasterizerState PointSpriteRasterizerState
{
	CullMode = NONE;
};

/**
 * ビルボードを生成するを生成するジオメトリシェーダーから利用する共通関数
 *
 * input  : 0
 *
 * output : 1--3
 *          |  |
 *          0--2
 */
[maxvertexcount(4)]
void gs_particle( point COMMON_POS_COLOR input[ 1 ], inout TriangleStream<COMMON_POS_COLOR> Stream )
{
	float vw = 0.1f;
	
	COMMON_POS_COLOR output[ 4 ];
	
	output[ 0 ].Position = input[ 0 ].Position + float4( -vw, -vw, 0.f, 0.f );
	output[ 1 ].Position = input[ 0 ].Position + float4( -vw,  vw, 0.f, 0.f );
	output[ 2 ].Position = input[ 0 ].Position + float4(  vw, -vw, 0.f, 0.f );
	output[ 3 ].Position = input[ 0 ].Position + float4(  vw,  vw, 0.f, 0.f );
	
	for ( int n = 0; n < 4; n++ )
	{
		output[ n ].Position = common_p_pos( output[ n ].Position );
		output[ n ].Color = input[ 0 ].Color;
	}
	
	Stream.Append( output[ 0 ] );
	Stream.Append( output[ 1 ] );
	Stream.Append( output[ 2 ] );
	Stream.Append( output[ 3 ] );
}

float4 ps_point_sprite( COMMON_POS_COLOR input ) : SV_Target
{
	return input.Color;
}

technique11 point_sprite
{
	pass main
	{
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( NoDepthTest, 0xFFFFFFFF );
		
		SetVertexShader( CompileShader( vs_4_0, vs_point_sprite() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( CompileShader( gs_4_0, gs_particle() ) );
		SetPixelShader( CompileShader( ps_4_0, ps_point_sprite() ) );

		// RASTERIZERSTATE = PointSpriteRasterizerState;
	}
}