-- CanvasTestScene �ɐ؂�ւ���
scene( "canvas_test" )

-- Plane �I�u�W�F�N�g�𐶐������O��t����
plane = create_object( "static" )
set_name( plane, "paint_guide_plane" )

-- Plane ���f����ǂݍ���� Plane �I�u�W�F�N�g�ɐݒ肷��
plane.model = load_model( "plane" )

-- UnicolorShader ���擾�� Plane ���f���ɐݒ肷��
s = get_shader( "unicolor" )
plane.model:set_shader_at( 0, s )

-- Plane �I�u�W�F�N�g�̈ʒu����]��ݒ肷��
set_loc( plane, 0, 0, 0 )
set_start_rot( plane, -90, 0, 0 )