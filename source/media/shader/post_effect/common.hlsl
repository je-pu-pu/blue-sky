Texture2D source_texture : register( t0 ); /// ポストエフェクト適用前のレンダリング結果が書き込まれているテクスチャ

/**
 * ポストエフェクト用共通 Vertex Shader
 */
COMMON_POS_UV vs_post_effect( COMMON_POS_NORM_UV input )
{
	COMMON_POS_UV output;

	output.Position = input.Position;
	output.TexCoord = input.TexCoord;

	return output;
}