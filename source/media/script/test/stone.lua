-- set_sky_box( "sky-box-3" )
-- set_ground( "ground" )

for y = 0, 10 - 1 do
	for x = 0, 10 - 1 do
		s = create_object( "stone" )
		set_loc( s, x, y, 0 )
	end
end
