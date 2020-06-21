-- シェーダーを切り替えるテスト

set_sky_box( "sky-box-3" )
set_ground( "ground" )

g1 = create_object( "girl" )
set_loc( g1, -0.5, 0, 0 )
set_dir( g1, 180 )

g2 = clone_object( g1 )
g2.model = clone_model( g2.model )
set_loc( g2, 0.5, 0, 0 )

-- s = get_shader( "matcap_skin" )
s = get_shader( "tess_matcap_skin" )
s1 = clone_shader( s )
s2 = clone_shader( s )

t1 = load_texture( "media/texture/matcap/test/a.png" )
t2 = load_texture( "media/texture/matcap/skin.png" )

s1:set_texture_at( 0, t1 )
g1.model:set_shader_at( 0, s1 )
g1.model:set_shader_at( 2, s1 )

s2:set_texture_at( 0, t2 )
g2.model:set_shader_at( 0, s2 )
g2.model:set_shader_at( 2, s2 )


function switch_texture( file_name )
	s1:set_texture_at( 0, load_texture( "media/texture/matcap/" .. file_name ) )
end