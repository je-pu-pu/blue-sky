set_sky_box( "sky-box-3" )
set_ground( "ground" )

girl = create_object( "girl" )
set_loc( girl, -0.5, 0, 0 )
set_dir( girl, 180 )

girl2 = clone_object( girl )
set_loc( girl2, 0.5, 0, 0 )

-- model = clone_model( girl2.model )
-- model:set_shader_at( 0, matcap )
-- girl2.model = model

flat = get_shader( "flat_skin" )
matcap = get_shader( "matcap_skin" )

set_shader( girl,  0, flat )
set_shader( girl2, 0, matcap )

-- texture = get_texture( "" )
-- set_shader_texture( matcap, 0, texture );