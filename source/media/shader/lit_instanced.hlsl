/**
 * インスタンシング対応シェーダー
 * 複数のオブジェクトを1回のドローコールで描画する
 */

// インスタンスごとのワールド行列を格納する構造体
struct InstanceData
{
	row_major matrix World;
};

// インスタンスごとのワールド行列を格納するStructuredBuffer
StructuredBuffer<InstanceData> InstanceWorldMatrices : register( t10 );

/**
 * インスタンシング対応の頂点シェーダー入力
 */
struct VS_INSTANCED_INPUT
{
	float4 Position : POSITION;
	float3 Normal   : NORMAL0;
	float2 TexCoord : TEXCOORD0;
	uint InstanceID : SV_InstanceID;
};

/**
 * インスタンシング対応のシェーディングありの頂点シェーダー
 */
COMMON_POS_UV_COLOR vs_lit_instanced( VS_INSTANCED_INPUT input )
{
	COMMON_POS_UV_COLOR output;

	// インスタンスごとのワールド行列を取得
	matrix instanceWorld = InstanceWorldMatrices[ input.InstanceID ].World;

	// WVP 変換
	float4 worldPos = mul( input.Position, instanceWorld );
	float4 viewPos = mul( worldPos, View );
	output.Position = mul( viewPos, Projection );

	output.TexCoord = input.TexCoord;

	// 法線をワールド座標系に変換してライティング計算
	float3 worldNormal = mul( input.Normal, ( float3x3 ) instanceWorld );
	output.Color = dot( -Light, worldNormal ) * 0.5f + 1.f;

	return output;
}

/**
 * インスタンシング対応のシェーディングありのピクセルシェーダー
 */
float4 ps_lit_instanced( COMMON_POS_UV_COLOR input ) : SV_Target
{
	// ビート同期の明るさ (BeatProgress: 1.0 -> 0.0)
	float beatBrightness = 1.0f + pow( BeatProgress, 0.5f ) * 0.5f;

	float4 color = model_texture.Sample( wrap_texture_sampler, input.TexCoord ) * input.Color;
	color.rgb *= beatBrightness;

	return color;
}

// シェーディングあり・インスタンシング
technique11 lit_instanced
{
	pass main
    {
		SetBlendState( NoBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( WriteDepth, 0xFFFFFFFF );

        SetVertexShader( CompileShader( vs_4_0, vs_lit_instanced() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, ps_lit_instanced() ) );

		RASTERIZERSTATE = Default;
    }
}
