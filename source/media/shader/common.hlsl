/**
 * デバッグ用の線のための DepthStencilState
 *
 */
DepthStencilState DebugLineDepthStencilState
{
	DepthWriteMask = ALL;
	DepthFunc = LESS_EQUAL;
};



/***
 * 座標のみを持つ共通頂点構造
 *
 */
struct COMMON_POS
{
	float4 Position : SV_POSITION;
};

/***
 * 座標と法線を持つ共通頂点構造
 *
 */
struct COMMON_POS_NORM
{
	float4 Position : SV_POSITION;
	float3 Normal   : NORMAL0;
};

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
 * 座標とテクスチャ UV 座標と色を持つ共通頂点構造
 *
 */
struct COMMON_POS_UV_COLOR
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
	float4 Color    : COLOR0;
};

float4 common_wv_pos( float4 input )
{
	return mul( mul( input, World ), View );
}

float4 common_wvp_pos( float4 input )
{
	return mul( mul( mul( input, World ), View ), Projection );
}

float4 common_vp_pos( float4 input )
{
	return mul( mul( input, View ), Projection );
}

float3 common_w_norm( float3 input )
{
	return mul( input, ( float3x3 ) World );
}

float3 common_v_norm( float3 input )
{
	return mul( input, ( float3x3 ) View );
}

float3 common_wv_norm( float3 input )
{
	return mul( mul( input, ( float3x3 ) World ), ( float3x3 ) View );
}

/**
 * シャドウマップのテクスチャ座標を返す
 *
 * @param pos ローカル座標系での頂点座標
 * @param csm_level カスケードレベル
 * @todo 理解する
 */
float4 common_shadow_texcoord( float4 pos, int csm_level )
{
	float4 shadow_texcoord = pos;

	shadow_texcoord = mul( shadow_texcoord, World );
	shadow_texcoord = mul( shadow_texcoord, ShadowViewProjection[ csm_level ] );
		
	shadow_texcoord /= shadow_texcoord.w;
	
	shadow_texcoord.x = (  shadow_texcoord.x + 1.f ) / 2.f;
	shadow_texcoord.y = ( -shadow_texcoord.y + 1.f ) / 2.f;

	return shadow_texcoord;
}

/**
 * シャドウマップをサンプリングするカスケードレベルを取得する
 *
 * @param input_depth 現在のピクセルのビュー座標系での Z 値
 * @return カスケードレベル ( 0 .. 3 )
 */
int common_shadow_cascade_index( float input_depth )
{
	const float4 comp = input_depth > ShadowMapViewDepthPerCascadeLevel;

	float cascade_index =
		dot(
			float4(
				ShadowMapCascadeLevels > 0,
				ShadowMapCascadeLevels > 1,
				ShadowMapCascadeLevels > 2,
				ShadowMapCascadeLevels > 3
			),
			comp
		);

	return min( cascade_index, ShadowMapCascadeLevels - 1 );
}

/**
 * シャドウマップをサンプリングし現在のピクセルが影かどうかを bool で返す
 *
 * @param shadow_tex_coords ???
 * @param input_depth 現在のピクセルのビュー座標系での Z 値
 * @todo 理解する
 */
bool common_sample_is_shadow( float4 shadow_tex_coords[ ShadowMapCascadeLevels ], float input_depth )
{
	const float cascade_index = common_shadow_cascade_index( input_depth );
	float4 shadow_tex_coord = shadow_tex_coords[ ( int ) cascade_index ];

	shadow_tex_coord.x /= ( float ) ShadowMapCascadeLevels;
	shadow_tex_coord.x += cascade_index / ShadowMapCascadeLevels;

	float4 depth = shadow_texture.Sample( shadow_texture_sampler, shadow_tex_coord.xy );

	// const float vsm_variance = depth.y - pow( depth.x, 2 );
	// const float vsm_p = vsm_variance / ( vsm_variance + pow( shadow_tex_coord.z - depth.x, 2 ) );
	// const float vsm_shadow_rate = 1.f - saturate( max( vsm_p, depth.x <= shadow_tex_coord.z ) );

	return shadow_tex_coord.z >= depth.x;
}

/**
 * 拡散を計算する
 * 
 * @param norm ワールド座標系での法線
 */
float common_diffuse( float3 norm )
{
	return ( 1.f - ( dot( norm, ( float3 ) Light ) * 0.5f + 0.5f ) );
}

/**
 * 頂点に対してスキニングを行う共通関数
 *
 */
float4 common_skinning_pos( float4 pos, uint4 bone_index, float4 weight )
{
	return 
		mul( pos, BoneMatrix[ bone_index.x ] ) * weight.x +
		mul( pos, BoneMatrix[ bone_index.y ] ) * weight.y +
		mul( pos, BoneMatrix[ bone_index.z ] ) * weight.z +
		mul( pos, BoneMatrix[ bone_index.w ] ) * weight.w;
}

/**
 * 法線に対してスキニングを行う共通関数
 *
 */
float3 common_skinning_norm( float3 norm, uint4 bone_index, float4 weight )
{
	return
		mul( norm, ( float3x3 ) BoneMatrix[ bone_index.x ] ) * weight.x +
		mul( norm, ( float3x3 ) BoneMatrix[ bone_index.y ] ) * weight.y +
		mul( norm, ( float3x3 ) BoneMatrix[ bone_index.z ] ) * weight.z +
		mul( norm, ( float3x3 ) BoneMatrix[ bone_index.w ] ) * weight.w;
}

/**
 * Matcap によるサンプリングを行う共通関数
 *
 * @param ビュー座標系に変換された法線
 */
float4 common_sample_matcap( float3 norm )
{
	float2 uv = float2( norm.x * 0.5f + 0.5, 1.f - ( norm.y * 0.5f + 0.5f ) );
	return matcap_texture.Sample( texture_sampler, uv );
}

/**
 * World * View * Proejction を COMMON_POS_NORM_UV に適用し COMMON_POS を出力する
 *
 */
COMMON_POS vs_common_wvp_pos_norm_uv_to_pos( COMMON_POS_NORM_UV input )
{
	COMMON_POS output;

	output.Position = common_wvp_pos( input.Position );

	return output;
}

/**
 * World * View * Proejction を COMMON_POS_NORM_UV に適用し COMMON_POS_UV を出力する
 *
 */
COMMON_POS_UV vs_common_wvp_pos_norm_uv_to_pos_uv( COMMON_POS_NORM_UV input )
{
	COMMON_POS_UV output;

	output.Position = common_wvp_pos( input.Position );
	output.TexCoord = input.TexCoord;

	return output;
}

/**
 * World * View * Proejction を COMMON_POS に適用し COMMON_POS を出力するバーテックスシェーダー
 *
 */
float4 vs_common_wvp_pos_to_pos( float4 input ) : SV_Position
{
	return common_wvp_pos( input );
}

/**
 * World * View * Proejction をスキンメッシュに適用し COMMON_POS を出力するバーテックスシェーダー
 *
 */
float4 vs_common_wvp_skin_to_pos( VS_SKIN_INPUT input ) : SV_Position
{
	return common_wvp_pos( common_skinning_pos( input.Position, input.Bone, input.Weight ) );
}

/***
 *
 *
 */
float4 ps_common_diffuse_pos_norm( COMMON_POS_NORM input ) : SV_Target
{
	const float diffuse = common_diffuse( input.Normal );
	return float4( diffuse, diffuse, diffuse, 1.f );
}

float4 ps_common_diffuse_pos_norm_uv( COMMON_POS_NORM_UV input ) : SV_Target
{
	const float3 normal = input.Normal;
	const float diffuse = common_diffuse( normal );
	return float4( diffuse, diffuse, diffuse, 1.f );
}

float4 ps_common_normal_map_pos_norm_uv( COMMON_POS_NORM_UV input ) : SV_Target
{
	// return float4( common_w_norm( normal_texture.Sample( texture_sampler, input.TexCoord ) ), 1.f );

	// const float3 normal = common_w_norm( input.Normal + 2.f * normal_texture.Sample( texture_sampler, input.TexCoord ) - 1.f );
	const float3 normal = input.Normal + 2.f * normal_texture.Sample( texture_sampler, input.TexCoord ).rgb - 1.f;
	const float diffuse = common_diffuse( normal );
	return float4( diffuse, diffuse, diffuse, 1.f );
}

float4 ps_common_sample_matcap_pos_norm( COMMON_POS_NORM input ) : SV_Target
{
	return common_sample_matcap( common_v_norm( input.Normal ) );
}

float4 ps_common_sample_matcap_pos_norm_uv( COMMON_POS_NORM_UV input ) : SV_Target
{
	/// @todo 修正する
	// return common_sample_matcap( common_v_norm( input.Normal + normal_texture.Sample( texture_sampler, input.TexCoord ) ) );
	return common_sample_matcap( input.Normal );
}

float4 ps_common_sample_pos_norm_uv( COMMON_POS_NORM_UV input ) : SV_Target
{
	return model_texture.Sample( wrap_texture_sampler, input.TexCoord );
}

/**
 * デバッグ用の輪郭線を描画するピクセルシェーダー
 *
 */
float4 ps_common_debug_line_pos( float4 input : SV_Position ) : SV_Target
{
	return float4( 1.f, 0.f, 0.f, 1.f );
}

/**
 * デバッグ用の輪郭線を描画するピクセルシェーダー
 *
 */
float4 ps_common_debug_line_pos_norm( COMMON_POS_NORM input ) : SV_Target
{
	return float4( 1.f, 0.f, 0.f, 1.f );
}