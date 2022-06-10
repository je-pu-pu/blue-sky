/**
 * �f�o�b�O�p�̐��̂��߂� DepthStencilState
 *
 */
DepthStencilState DebugLineDepthStencilState
{
	DepthWriteMask = ALL;
	DepthFunc = LESS_EQUAL;
};



/***
 * ���W�݂̂������ʒ��_�\��
 *
 */
struct COMMON_POS
{
	float4 Position : SV_POSITION;
};

/***
 * ���W�Ɩ@���������ʒ��_�\��
 *
 */
struct COMMON_POS_NORM
{
	float4 Position : SV_POSITION;
	float3 Normal   : NORMAL0;
};

/***
 * ���W�Ɩ@���ƃe�N�X�`�� UV ���W�������ʒ��_�\��
 *
 */
struct COMMON_POS_NORM_UV
{
	float4 Position : SV_POSITION;
	float3 Normal   : NORMAL0;
	float2 TexCoord : TEXCOORD0;
};

/**
 * ���W�ƃe�N�X�`�� UV ���W�������ʒ��_�\��
 *
 */
struct COMMON_POS_UV
{
	float4 Position : SV_Position;
	float2 TexCoord : TEXCOORD0;
};

/**
 * ���W�ƃe�N�X�`�� UV ���W�ƐF�������ʒ��_�\��
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
 * �V���h�E�}�b�v�̃e�N�X�`�����W��Ԃ�
 *
 * @param pos ���[�J�����W�n�ł̒��_���W
 * @param csm_level �J�X�P�[�h���x��
 * @todo ��������
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
 * �V���h�E�}�b�v���T���v�����O����J�X�P�[�h���x�����擾����
 *
 * @param input_depth ���݂̃s�N�Z���̃r���[���W�n�ł� Z �l
 * @return �J�X�P�[�h���x�� ( 0 .. 3 )
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
 * �V���h�E�}�b�v���T���v�����O�����݂̃s�N�Z�����e���ǂ����� bool �ŕԂ�
 *
 * @param shadow_tex_coords ???
 * @param input_depth ���݂̃s�N�Z���̃r���[���W�n�ł� Z �l
 * @todo ��������
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
 * �g�U���v�Z����
 * 
 * @param norm ���[���h���W�n�ł̖@��
 */
float common_diffuse( float3 norm )
{
	return ( 1.f - ( dot( norm, ( float3 ) Light ) * 0.5f + 0.5f ) );
}

/**
 * ���_�ɑ΂��ăX�L�j���O���s�����ʊ֐�
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
 * �@���ɑ΂��ăX�L�j���O���s�����ʊ֐�
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
 * Matcap �ɂ��T���v�����O���s�����ʊ֐�
 *
 * @param �r���[���W�n�ɕϊ����ꂽ�@��
 */
float4 common_sample_matcap( float3 norm )
{
	float2 uv = float2( norm.x * 0.5f + 0.5, 1.f - ( norm.y * 0.5f + 0.5f ) );
	return matcap_texture.Sample( texture_sampler, uv );
}

/**
 * World * View * Proejction �� COMMON_POS_NORM_UV �ɓK�p�� COMMON_POS ���o�͂���
 *
 */
COMMON_POS vs_common_wvp_pos_norm_uv_to_pos( COMMON_POS_NORM_UV input )
{
	COMMON_POS output;

	output.Position = common_wvp_pos( input.Position );

	return output;
}

/**
 * World * View * Proejction �� COMMON_POS_NORM_UV �ɓK�p�� COMMON_POS_UV ���o�͂���
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
 * World * View * Proejction �� COMMON_POS �ɓK�p�� COMMON_POS ���o�͂���o�[�e�b�N�X�V�F�[�_�[
 *
 */
float4 vs_common_wvp_pos_to_pos( float4 input ) : SV_Position
{
	return common_wvp_pos( input );
}

/**
 * World * View * Proejction ���X�L�����b�V���ɓK�p�� COMMON_POS ���o�͂���o�[�e�b�N�X�V�F�[�_�[
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
	/// @todo �C������
	// return common_sample_matcap( common_v_norm( input.Normal + normal_texture.Sample( texture_sampler, input.TexCoord ) ) );
	return common_sample_matcap( input.Normal );
}

float4 ps_common_sample_pos_norm_uv( COMMON_POS_NORM_UV input ) : SV_Target
{
	return model_texture.Sample( wrap_texture_sampler, input.TexCoord );
}

/**
 * �f�o�b�O�p�̗֊s����`�悷��s�N�Z���V�F�[�_�[
 *
 */
float4 ps_common_debug_line_pos( float4 input : SV_Position ) : SV_Target
{
	return float4( 1.f, 0.f, 0.f, 1.f );
}

/**
 * �f�o�b�O�p�̗֊s����`�悷��s�N�Z���V�F�[�_�[
 *
 */
float4 ps_common_debug_line_pos_norm( COMMON_POS_NORM input ) : SV_Target
{
	return float4( 1.f, 0.f, 0.f, 1.f );
}