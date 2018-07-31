COMMON_POS_NORM_UV vs_nop_pos_norm_uv( COMMON_POS_NORM_UV input )
{
	COMMON_POS_NORM_UV output;

	output.Position = input.Position;
	output.Normal = input.Normal;
	output.TexCoord = input.TexCoord;

	// output.Position = common_wvp_pos( output.Position );
	// output.Normal = common_w_norm( output.Normal );

	return output;
}

COMMON_POS_NORM_UV vs_skin_tess_test( VS_SKIN_INPUT input )
{
	COMMON_POS_NORM_UV output;

	output.Position = common_skinning_pos( input.Position, input.Bone, input.Weight );
	output.Position /= output.Position.w; /// @todo なぜ必要なのか調べる

	output.Normal = common_skinning_norm( input.Normal, input.Bone, input.Weight );

	output.TexCoord = input.TexCoord;
	
	return output;
}

struct HS_CONSTANT_OUTPUT
{
	float factor[ 3 ]  : SV_TessFactor;
	float inner_factor : SV_InsideTessFactor;
};

HS_CONSTANT_OUTPUT hs_constant( InputPatch<COMMON_POS_NORM_UV, 3> input )
{
	HS_CONSTANT_OUTPUT Out;

	float devide = TessFactor;

	Out.factor[ 0 ] = devide;
	Out.factor[ 1 ] = devide;
	Out.factor[ 2 ] = devide;

	Out.inner_factor = devide;

	return Out;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("hs_constant")]
COMMON_POS_NORM_UV hs_phong_tessellation( InputPatch<COMMON_POS_NORM_UV, 3> ip, uint cpid : SV_OutputControlPointID )
{
	return ip[ cpid ];
}

/**
 * pos を patch_pos から patch_norm 方向に向かうベクトルに対し射影し、その長さだけ pos を patch_norm 方向に移動したベクトルを返す
 * 
 * 
 */
float3 common_project_pos_to_tangent_space( float3 pos, float3 patch_pos, float3 patch_norm )
{
	const float length = dot( ( patch_pos - pos ), patch_norm );
	return pos + patch_norm * length * 0.5f;
}

/**
 * フォンテッセレーション
 *
 */
float4 common_phong_tessellation( const OutputPatch<COMMON_POS_NORM_UV, 3> patch, float3 uvw )
{
	const float4 pos = patch[ 0 ].Position * uvw.x + patch[ 1 ].Position * uvw.y + patch[ 2 ].Position * uvw.z;	
	float3 output_pos = float3( 0.f, 0.f, 0.f );

    for ( int n = 0; n < 3; n++ )
    {
        output_pos += common_project_pos_to_tangent_space( pos.xyz, patch[ n ].Position.xyz, patch[ n ].Normal.xyz ) * uvw[ n ];
    }

	return float4( output_pos, 1.f );
}

[domain("tri")]
COMMON_POS_NORM_UV ds_phong_tessellation( HS_CONSTANT_OUTPUT input, float3 uvw : SV_DomaInLocation, const OutputPatch<COMMON_POS_NORM_UV, 3> patch )
{
	COMMON_POS_NORM_UV output;

	output.Normal   = patch[ 0 ].Normal   * uvw.x + patch[ 1 ].Normal   * uvw.y + patch[ 2 ].Normal   * uvw.z;
	output.TexCoord = patch[ 0 ].TexCoord * uvw.x + patch[ 1 ].TexCoord * uvw.y + patch[ 2 ].TexCoord * uvw.z;

	// phong tessellation
	output.Position = common_phong_tessellation( patch, uvw );

	// 
	output.Position = common_wvp_pos( output.Position );
	output.Normal = common_wv_norm( output.Normal );

	return output;
}

[domain("tri")]
COMMON_POS_NORM_UV ds_displacement( HS_CONSTANT_OUTPUT input, float3 uvw : SV_DomaInLocation, const OutputPatch<COMMON_POS_NORM_UV, 3> patch )
{
	COMMON_POS_NORM_UV output;

	output.Position = patch[ 0 ].Position * uvw.x + patch[ 1 ].Position * uvw.y + patch[ 2 ].Position * uvw.z;
	output.Normal   = patch[ 0 ].Normal   * uvw.x + patch[ 1 ].Normal   * uvw.y + patch[ 2 ].Normal   * uvw.z;
	output.TexCoord = patch[ 0 ].TexCoord * uvw.x + patch[ 1 ].TexCoord * uvw.y + patch[ 2 ].TexCoord * uvw.z;
	
	// displacement mapping
	const float height = displacement_texture.SampleLevel( texture_sampler, output.TexCoord, 0 ).x;
	output.Position += float4( output.Normal * ( height - 0.5f ), 0 );

	// 
	output.Position = common_wvp_pos( output.Position );
	output.Normal = common_wv_norm( output.Normal );

	return output;
}

technique11 tessellation
{
	pass tessellation
	{
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_nop_pos_norm_uv() ) );
		SetHullShader( CompileShader( hs_5_0, hs_phong_tessellation() ) );
		SetDomainShader( CompileShader( ds_5_0, ds_phong_tessellation() ) );
		// SetDomainShader( CompileShader( ds_5_0, ds_displacement() ) );
		SetGeometryShader( NULL );
		// SetPixelShader( CompileShader( ps_4_0, ps_common_sample_pos_norm_uv() ) );
		// SetPixelShader( CompileShader( ps_4_0, ps_common_diffuse_pos_norm() ) );
		// SetPixelShader( CompileShader( ps_4_0, ps_common_diffuse_pos_norm_uv() ) );
		SetPixelShader( CompileShader( ps_4_0, ps_common_normal_map_pos_norm_uv() ) );
		// SetPixelShader( CompileShader( ps_4_0, ps_common_sample_matcap_pos_norm_uv() ) );

		RASTERIZERSTATE = Default;
	}

	/*
	pass debug_line
    {
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( DebugLineDepthStencilState, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_nop_pos_norm_uv() ) );
		SetHullShader( CompileShader( hs_5_0, hs_phong_tessellation() ) );
		SetDomainShader( CompileShader( ds_5_0, ds_phong_tessellation() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_common_debug_line_pos_norm() ) );

		RASTERIZERSTATE = WireframeRasterizerState;
    }
	*/
}

technique11 tessellation_skin
{
	pass tessellation
	{
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_skin_tess_test() ) );
		SetHullShader( CompileShader( hs_5_0, hs_phong_tessellation() ) );
		SetDomainShader( CompileShader( ds_5_0, ds_phong_tessellation() ) );
		SetGeometryShader( NULL );
		// SetPixelShader( CompileShader( ps_4_0, ps_common_diffuse_pos_norm_uv() ) );
		SetPixelShader( CompileShader( ps_4_0, ps_common_sample_matcap_pos_norm_uv() ) );

		RASTERIZERSTATE = Default;
	}

	/*
	pass debug_line
    {
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( DebugLineDepthStencilState, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_skin_tess_test() ) );
		SetHullShader( CompileShader( hs_5_0, hs_phong_tessellation() ) );
		SetDomainShader( CompileShader( ds_5_0, ds_phong_tessellation() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, ps_common_debug_line_pos_norm() ) );

		RASTERIZERSTATE = WireframeRasterizerState;
    }
	*/
}
