cbuffer GameConstantBuffer : register( b10 )
{
	float4 UnicolorObjectColor;
};

float4 ps_unicolor( COMMON_POS input ) : SV_Target
{
	 return UnicolorObjectColor;
}

technique11 unicolor
{
	pass main
    {
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

        SetVertexShader( CompileShader( vs_4_0, vs_common_wvp_pos_norm_uv_to_pos() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_unicolor() ) );

		RASTERIZERSTATE = Default;
    }
}

// 単色 ＋ 面ごとの陰影（法線ライティング）。ビル等の塗りつぶしに使う。
COMMON_POS_UV_COLOR vs_lit_unicolor( COMMON_POS_NORM_UV input )
{
	COMMON_POS_UV_COLOR output;

	output.Position = common_wvp_pos( input.Position );
	output.TexCoord = input.TexCoord;
	output.Color = dot( -Light.xyz, input.Normal ) * 0.5f + 0.75f; // 面の向きで陰影（0.25..1.25）

	return output;
}

float4 ps_lit_unicolor( COMMON_POS_UV_COLOR input ) : SV_Target
{
	float4 c = UnicolorObjectColor;
	c.rgb *= input.Color.rgb; // 単色 × 陰影
	return c;
}

technique11 lit_unicolor
{
	pass main
    {
		SetBlendState( NoBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

        SetVertexShader( CompileShader( vs_4_0, vs_lit_unicolor() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_lit_unicolor() ) );

		RASTERIZERSTATE = Default;
    }
}
