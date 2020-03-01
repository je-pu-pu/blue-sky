set_sky_box( "sky-box-3" )
-- set_ground( "ground" )

robot = create_object( "robot" )
set_loc( robot, 2.5, 0, 0 )
set_dir( robot, 180 )

girl = create_object( "girl" )
set_loc( girl, -2.5, 0, 0 )
set_dir( girl, 180 )


-- 
-- load( "test/switch_shader.lua" )


-- FBX ��ǂݍ���ŕ\������
-- FBX �Ƀe�N�X�`�����ݒ肳��Ă��Ȃ��Ă��X�N���v�g�Ńe�N�X�`����ݒ�ł���

-- �e�N�X�`����ǂݍ���
-- t = load_texture( "media/texture/matcap/test/test.png" )

-- �t���b�g�V�F�[�_�[���w�肵�ăe�N�X�`�����w�肷��
s = get_shader( "flat" )

t = s:get_texture_at( 0 )
s:set_texture_at( 0, t )

-- �X�^�e�B�b�N�ȃI�u�W�F�N�g���쐬����
o = create_object( "static" )
set_loc( o, 0, 1, 0 )

-- ���f���� FBX ����ǂݍ���ŃV�F�[�_�[��ݒ肷��
o.model = load_model( "sphere" )
o.model:set_shader_at( 0, s )

