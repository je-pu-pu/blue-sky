/***
 * 座標と法線とテクスチャ UV 座標を持つ共通頂点構造
 *
 */
struct COMMON_POS_NORM_UV
{
	float4 Position : SV_POSITION;
	float3 Normal   : NORMAL0;
	float2 TexCoord : TEXCOORD0;
};

/**
 * 座標とテクスチャ UV 座標を持つ共通頂点構造
 *
 */
struct COMMON_POS_UV
{
	float4 Position : SV_Position;
	float2 TexCoord : TEXCOORD0;
};

/**
 * World * View * Proejction を COMMON_POS_NORM_UV に適用し COMMON_POS_UV を出力する
 *
 */
COMMON_POS_UV vs_common_wvp_pos_norm_uv_to_pos_uv( COMMON_POS_NORM_UV input )
{
	COMMON_POS_UV output;

	output.Position = mul( input.Position, World );
    output.Position = mul( output.Position, View );
    output.Position = mul( output.Position, Projection );

	output.TexCoord = input.TexCoord;

	return output;
}

/***
 *
 *
 */
float4 ps_common_( PS_FLAT_INPUT input ) : SV_Target
{
	return model_texture.Sample( texture_sampler, input.TexCoord ) * ObjectColor;
}