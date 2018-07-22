set_sky_box( "sky-box-3" )
set_ground( "ground" )

girl = create_object( "girl" )
set_loc( girl, -0.5, 0, 0 )
set_dir( girl, 180 )

girl2 = clone_object( girl )
set_loc( girl2, 0.5, 0, 0 )
set_dir( girl2, 180 )