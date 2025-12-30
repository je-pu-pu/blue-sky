/**
 * インスタンシング対応フラットシェーダー
 * 複数のオブジェクトを1回のドローコールで描画する（シェーディングなし）
 *
 * 注意: lit_instanced.hlsl の後に include されることを前提とし、
 *       InstanceWorldMatrices と VS_INSTANCED_INPUT はそちらで定義済み
 */

/**
 * インスタンシング対応のフラットシェーダー頂点シェーダー
 */
COMMON_POS_UV_COLOR vs_flat_instanced( VS_INSTANCED_INPUT input )
{
	COMMON_POS_UV_COLOR output;

	// インスタンスごとのワールド行列を取得
	float4x4 instanceWorld = InstanceWorldMatrices[ input.InstanceID ].World;

	// WVP 変換
	float4 worldPos = mul( input.Position, instanceWorld );
	float4 viewPos = mul( worldPos, View );
	output.Position = mul( viewPos, Projection );

	output.TexCoord = input.TexCoord;
	output.Color = ObjectColor;

	return output;
}

/**
 * インスタンシング対応のフラットシェーダーピクセルシェーダー
 */
float4 ps_flat_instanced( COMMON_POS_UV_COLOR input ) : SV_Target
{
	return model_texture.Sample( wrap_texture_sampler, input.TexCoord );
}

// フラット・インスタンシング
technique11 flat_instanced
{
	pass main
    {
		SetBlendState( NoBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

        SetVertexShader( CompileShader( vs_4_0, vs_flat_instanced() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, ps_flat_instanced() ) );

		RASTERIZERSTATE = Default;
    }
}
