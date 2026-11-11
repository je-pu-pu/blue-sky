/**
 * シャドウマップへのレンダリング
 *
 */
float4 vs_shadow_map_common( float4 position, matrix world )
{
	return mul( common_mul_world( position, world ), ShadowViewProjection[ 0 ] );
}

float4 vs_shadow_map( COMMON_POS_NORM_UV input ) : SV_POSITION
{
	return vs_shadow_map_common( input.Position, World );
}

/**
 * シャドウマップへのレンダリング（同じモデルの物体をまとめて描く版。物体ごとの定数を object_instance.hlsl から読む）
 */
float4 vs_shadow_map_instanced( COMMON_POS_NORM_UV input, uint instance_id : SV_InstanceID ) : SV_POSITION
{
	return vs_shadow_map_common( input.Position, get_object_instance( instance_id ).World );
}

/**
 * 線のシャドウマップへのレンダリング（同じモデルの物体をまとめて描く版）
 *
 * 以前は線を、直前に当てたメッシュのシャドウマップのパス（vs_shadow_map・入力レイアウト "main"）のまま描いていた。
 * 位置だけを読むので、線の頂点（入力レイアウト "pos_color"）から位置を読んで同じ計算をする。
 * 線は幾何シェーダーで太らせず、線分のまま（1 画素の幅）シャドウマップに描く（以前と同じ）
 */
float4 vs_shadow_map_line_instanced( COMMON_POS_COLOR input, uint instance_id : SV_InstanceID ) : SV_POSITION
{
	return vs_shadow_map_common( input.Position, get_object_instance( instance_id ).World );
}

/**
 * シャドウマップへのスキンメッシュのレンダリング
 *
 */
float4 vs_shadow_map_skin( VS_SKIN_INPUT input ) : SV_POSITION
{
	float4 output = common_skinning_pos( input.Position, input.Bone, input.Weight );

	output = mul( output, World );
    output = mul( output, ShadowViewProjection[ 0 ] );

	return output;
}

technique11 shadow_map
{
	pass main
	{
		SetBlendState( NoBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_shadow_map() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
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
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( NULL );

		RASTERIZERSTATE = Shadow;
	}
}

// シャドウマップ（同じモデルの物体をまとめて描く版。頂点シェーダーのほかは shadow_map と同じ）
technique11 shadow_map_instanced
{
	pass main
	{
		SetBlendState( NoBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_shadow_map_instanced() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( NULL );

		RASTERIZERSTATE = Shadow;
	}
}

// 線のシャドウマップ（同じモデルの物体をまとめて描く版。入力レイアウト "pos_color"。頂点シェーダーのほかは shadow_map と同じ）
technique11 shadow_map_line_instanced
{
	pass main
	{
		SetBlendState( NoBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_shadow_map_line_instanced() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( NULL );

		RASTERIZERSTATE = Shadow;
	}
}