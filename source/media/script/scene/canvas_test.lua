-- CanvasTestScene に切り替える
scene( "canvas_test" )

-- Plane オブジェクトを生成し名前を付ける
plane = create_object( "static" )
set_name( plane, "paint_guide_plane" )

-- Plane モデルを読み込んで Plane オブジェクトに設定する
plane.model = load_model( "plane" )

-- UnicolorShader を取得し Plane モデルに設定する
s = get_shader( "unicolor" )
plane.model:set_shader_at( 0, s )

-- Plane オブジェクトの位置を回転を設定する
set_loc( plane, 0, 0, 0 )
set_start_rot( plane, -90, 0, 0 )