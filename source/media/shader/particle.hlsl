/**
 * ビルボードを生成するを生成するジオメトリシェーダーから利用する共通関数
 *
 * input  : 0
 *
 * output : 1--3
 *          |  |
 *          0--2
 */
void get_drawing_line_common( point COMMON_POS input[ 1 ], out COMMON_POS output[ 4 ] )
{
	float4 vw = 0.5f;

	output[ 0 ].Position = input[ 0 ].Position - vw;
	output[ 1 ].Position = input[ 0 ].Position + vw;
	output[ 2 ].Position = input[ 0 ].Position - vw;
	output[ 3 ].Position = input[ 0 ].Position + vw;
	}
}

technique11 particle
{
	pass main
	{
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( NoWriteDepth, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_common_wvp_pos_to_pos() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( CompileShader( gs_4_0, gs_particle() ) );
		SetPixelShader( CompileShader( ps_4_0, ps_common_debug_line_pos() ) );

		RASTERIZERSTATE = Default;
	}
}