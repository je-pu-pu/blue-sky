COMMON_POS_UV_COLOR vs_flat_common( COMMON_POS_NORM_UV input, matrix world, float4 color )
{
	COMMON_POS_UV_COLOR output;

	output.Position = common_wvp_pos_w( input.Position, world );
	output.TexCoord = input.TexCoord;
	output.Color = color;

	return output;
}

COMMON_POS_UV_COLOR vs_flat( COMMON_POS_NORM_UV input )
{
	return vs_flat_common( input, World, ObjectColor );
}

/**
 * vs_flat の、同じモデルの物体をまとめて描く版（物体ごとの定数を object_instance.hlsl から読む）
 *
 * 以前からある flat_instanced（lit_instanced.hlsl の world 行列だけの入れ物を使う。core の RenderSystem 用）とは別
 */
COMMON_POS_UV_COLOR vs_flat_object_instanced( COMMON_POS_NORM_UV input, uint instance_id : SV_InstanceID )
{
	const ObjectInstanceData object = get_object_instance( instance_id );

	return vs_flat_common( input, object.World, object.Color );
}

float4 ps_flat( COMMON_POS_UV_COLOR input ) : SV_Target
{
	float4 output = model_texture.Sample( texture_sampler, input.TexCoord ) * input.Color;

	// clip( output.a - 0.0001f );

	return output;
}

float4 ps_flat_with_flicker( COMMON_POS_UV_COLOR input ) : SV_Target
{
	return model_texture.Sample( texture_sampler, input.TexCoord + float2( sin( input.TexCoord.y * 50.f + TimeBeat ) * 0.01f, 0.f ) ) + input.Color;
}

float4 ps_main_wrap_flat( COMMON_POS_UV_COLOR input ) : SV_Target
{
	return model_texture.Sample( wrap_texture_sampler, input.TexCoord );
}

struct PS_PRIMITIVE_INPUT
{
	float4 Position : SV_POSITION;
	uint id : ID;
};

// test
PS_PRIMITIVE_INPUT vs_primitive_id( COMMON_POS_NORM_UV input, uint vertex_id : SV_VertexID )
{
	PS_PRIMITIVE_INPUT output;
	
	output.Position = common_wvp_pos( input.Position );
	output.id = vertex_id;

	return output;
}

uint ps_primitive_id( PS_PRIMITIVE_INPUT input ) : SV_Target
{
	return input.id;

	/*
	float4 colors[] = {
		{ 1.f, 0.f, 0.f, 1.f },
		{ 0.f, 1.f, 0.f, 1.f },
		{ 0.f, 0.f, 1.f, 1.f },
		{ 1.f, 1.f, 0.f, 1.f },
		{ 1.f, 0.f, 1.f, 1.f },
		{ 0.f, 1.f, 1.f, 1.f },
	};

	return colors[ input.id % 6 ];
	*/
}


// シェーディングなし・スキニングなし
technique11 flat
{
	pass main
    {
		SetBlendState( NoBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

        SetVertexShader( CompileShader( vs_4_0, vs_flat() ) );
		// SetVertexShader( CompileShader( vs_4_0, vs_primitive_id() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, ps_main_wrap_flat() ) );
		// SetPixelShader( CompileShader( ps_4_0, ps_flat_with_flicker() ) );
		// SetPixelShader( CompileShader( ps_4_0, ps_primitive_id() ) );

		RASTERIZERSTATE = Default;
    }

#if 0
	pass debug_line
    {
		SetBlendState( Blend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( DebugLineDepthStencilState, 0xFFFFFFFF );

		SetVertexShader( CompileShader( vs_4_0, vs_common_wvp_pos_norm_uv_to_pos() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_common_debug_line_pos() ) );

		RASTERIZERSTATE = WireframeRasterizerState;
    }
#endif
}

// シェーディングなし・スキニングなし（同じモデルの物体をまとめて描く版。頂点シェーダーのほかは flat と同じ）
technique11 flat_object_instanced
{
	pass main
    {
		SetBlendState( NoBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

        SetVertexShader( CompileShader( vs_4_0, vs_flat_object_instanced() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, ps_main_wrap_flat() ) );

		RASTERIZERSTATE = Default;
    }
}
