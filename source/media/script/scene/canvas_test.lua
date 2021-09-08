scene( "canvas_test" )

plane = create_object( "static" )
set_name( plane, "paint_guide_plane" )

plane.model = load_model( "plane" )

s = get_shader( "transparent" )

-- s = get_shader( "matcap" )
-- t = load_texture( "media/texture/matcap/test/a.png" )
-- s:set_texture_at( 0, t )

plane.model:set_shader_at( 0, s )

set_loc( plane, 0, 0, 0 )
set_start_rot( plane, -90, 0, 0 )