Texture2D source_texture : register( t0 ); /// �|�X�g�G�t�F�N�g�K�p�O�̃����_�����O���ʂ��������܂�Ă���e�N�X�`��

/**
 * �|�X�g�G�t�F�N�g�p���� Vertex Shader
 */
COMMON_POS_UV vs_post_effect( COMMON_POS_NORM_UV input )
{
	COMMON_POS_UV output;

	output.Position = input.Position;
	output.TexCoord = input.TexCoord;

	return output;
}