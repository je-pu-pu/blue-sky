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


-- FBX を読み込んで表示する
-- FBX にテクスチャが設定されていなくてもスクリプトでテクスチャを設定できる

-- テクスチャを読み込む
-- t = load_texture( "media/texture/matcap/test/test.png" )

-- フラットシェーダーを指定してテクスチャを指定する
s = get_shader( "flat" )

t = s:get_texture_at( 0 )
s:set_texture_at( 0, t )

-- スタティックなオブジェクトを作成する
o = create_object( "static" )
set_loc( o, 0, 1, 0 )

-- モデルを FBX から読み込んでシェーダーを設定する
o.model = load_model( "sphere" )
o.model:set_shader_at( 0, s )

