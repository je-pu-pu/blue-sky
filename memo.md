# blue-sky memo

-----

## マテリアル設定 ( 案 )

```c++
GameObject* object = get_game_object_manager()->get_nearest_object( cursor_pos );
auto* drawing_model = object->get_component< DrawingModel >();


Material* new_material = get_graphics_manager()->create_material();
new_material->set_shader( "" );
new_material->set_texture( "" );

drawing_model->get_mesh()->set_material_at( 0, new_material );
```

-----

## シェーダー ( 案 )

```c++
class MyShader : public Shader
{
public:
	MyShader::MyShader()
	{
		bind_to_vs< GameConstantBuffer >();
		bind_to_
	}
}
```

ゲームロジックから Bullet Physics を隠蔽する
    Player, Robot などから抽象化されたレイテスト・コンタクトテストを使えるようにする
    GameObject は剛体を触らないようにする？


-----

## 2018-09-18 シーングラフ上に矩形を作成するインターフェイス ( 案 )

```c++
void add_rectangle()
{
    auto* mesh = get_graphics_manager()->create_named_mesh< Rectangle >( "rectangle", Rectangle::Buffer::Type::DEFAULT );
    auto* shader = get_graphics_manager()->get_shader( "debug_shadow_map_texture" );

    auto* model = get_graphics_manager()->create_named_model( "rectangle" );
    model->set_mesh( mesh );
    model->set_shader_at( 0, shader );

    auto* object = new GameObject();
    object->set_model( model );
    object->transform

    get_scene_graph_root()->add_child( objec );
}
```

-----

## 2019-05-31 シェーダーグラフ

```c++
void MyScene::init()
{
    // ポストエフェクト用のシェーダーを生成する
    auto shader = MyPostEffectShader();

    // シェーダーのパラメータを変更する
    shader.set_my_param1( 123.f );
    shader.set_my_param2( 456 );

    // ポストエフェクト用のシェーダーを設定する
    get_graphics_manager().set_post_effect_shader( shader );
}
```

-----

2018-09-18 デプスバッファをテクスチャとして参照したい ( スクリーンスペースデカールを実現するため )
2018-09-18 マルチサンプリングされたリソースに対して、マルチサンプリング用でないビューを作成する事はできない

> D3D11 ERROR: ID3D11Device::CreateShaderResourceView: The base resource was created as a multisample resource.
> You must specify D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY or D3D11_SRV_DIMENSION_TEXTURE2DMS.
> [ STATE_CREATION ERROR #126: CREATESHADERRESOURCEVIEW_INVALIDDESC]

2018-09-19 参考 : Unity の場合 : Transform は Component を継承している。 Transform がシーングラフ階層構造を持つ

2018-09-20 デカールの実装のため [humus の VolumeDecal]( http://www.humus.name/index.php?page=3D&ID=83 ) を調査。しかしプログラム内でノイズ関数を使って生成した Texture3D を使った実装である事が判明。blue-sky では 2D の画像を読み込んでデカールとして使用したいため、参考にならず。

2018-09-20 [humus の Framework3]( http://www.humus.name/index.php?page=3D ) は、Direct3D, OpenGL 等のレンダーラーとアプリケーションをシンプルに抽象化していて参考になった。シェーダーはファイルを自前で開き、[VertexShader] [FragmentShader] などの文字列でシェーダー事にパースし、その後コンパイル関数に渡しているもよう。

# グラフィックスライブラリの抽象化

```c++

    // Factory を使うパターン
    void a()
    {
        // GraphicsManager がプラットフォームに合わせたテクスチャを生成する
        auto t = get_graphics_manager().createRenderTargetTexture( PixelFormat::R8G8B8_UNORM, 1024, 1024 );

        get_graphics_manager().set_render_target( t );
    }
    
    // a new を使うパターン
    void b()
    {
        // using RenderTargetTexture = direct_3d_11::RenderTargetTexture ？
        auto t = new RenderTargetTexture( PixelFormat::R8G8B8_UNORM, 1024, 1024 );
        
        // シーンをテクスチャに描画する
        get_graphics_manager().set_render_target( t );
        get_graphics_manager().render();

        // ポストエフェクト用のシェーダーを設定する
        get_graphics_manager().set_render_target( get_graphics_manager().get_back_buffer_texture() );
        get_graphics_manager().set_post_effect_shader( shader );
    }
```

## バックバッファへの直接描画

```c++
    void render()
    {
        // 描画対象をバックバッファに指定
        get_graphics_manager().set_render_target( get_graphics_manager().get_back_buffer_texture() ); 

        // 描画
        get_graphics_manager().render_background();
        get_graphics_manager().render_active_objects( get_active_object_manager() );
    }
```

## テクスチャへの描画

```c++

    RenderTargetTexture* t;

    void init()
    {
        t = get_graphics_manager().create_render_target_texture( get_graphics_manager().width(), get_graphics_manager().height() );
    }

    void render()
    {
        // 描画対象をテクスチャに指定
        get_graphics_manager().set_render_target( t ); 

        // テクスチャへ描画
        get_graphics_manager().render_background();
        get_graphics_manager().render_active_objects( get_active_object_manager() );

        // テクスチャからバックバッファに描画
        get_graphics_manager().render_post_effect();
    }
```


## Texture の持つ機能

| 機能               | 説明                                                         |
|--------------------|--------------------------------------------------------------|
| ShaderResourceView | モデルのレンダリング時に、シェーダーから利用できる           |
| RenderTargetView   | モデルのレンダリング時に、レンダリング結果を書き込める       |
| Texture2D          | 実際の 2D テクスチャのデータ ( への参照 )                    |

## 各 Texture が持つ機能

| 名前                | ShaderResourceView | RenderTargetView | Texture2D           |
|---------------------|--------------------|------------------|---------------------|
| Texture             | YSE                |                  | YES                 |
| RenderTargetTexture | YES                | YES              | YES                 |
| BackBufferTexture   |                    | YES              | YES/NO              |

# 2019-06-18 Texture 継承問題 課題

* ポストエフェクトを実現したい
* ポストエフェクトを実現するためにはレンダリング結果をテクスチャに書き込む必要がある
* 現状の Texture はシェーダーリソースとしてのみ使え、レンダリング結果は書き込めない
* シェーダーリソース + レンダーターゲットビューとして使えるテクスチャが必要
* 

# 2020-03-01 Texture 継承問題 解決案

以下の 2 つのインターフェイスを新設

* ShaderResource インターフェイス
* RenderTarget インターフェイス

# 2021-10-17

## core::graphics::direct_3d_11::ConstantBuffer について

* DataType ( 構造体 ) が無くても使えるようにしたい
    * そのためには以下の情報が必要
        * サイズ
        * SLOT 
* SLOT はコンパイル時に固定できるのでは？
    * **SetConstantBuffers() はスロットを指定できる = 実行時に変更できる

## 2021-10-18

ConstantBuffer には以下の 3 種類が欲しい。

* データの型は指定ぜず、データも保持しない ( 更新時に間違った型のデータを送ってしまう危険性がある )
    * ConstantBuffer
* データの型は指定する、データは保持しない ( )
    * ConstantBufferTyped
* データの型を指定し、データも保持する
    * ConstantBufferWithData

## 2021-10-20

* ConstantBuffer はどれぐらいの場所で使われているのか？
    * ConstantBuffer -> ConstantBufferTyped への置換

## 2021-10-22

* ShaderMixin を使う事で CanvasTestScene のプレーンの色を変更・保持する事に成功
    * 描画はまだ未対応

## 2021-10-24

* 暫定的に Model::render() の中で Shader::update() が必要？
* Shader から ConstantBuffer の update() が必要

## 2021-10-26

* ShaderMixin から ConstantBuffer の更新。
* 設計が綺麗かは置いておいて、とりあえず動く事を目指す。

## 2021-10-27

* unicolor.hlsl を実装。
* CanvasTestScene で UI で色を変えるとプレーンの色が変わるようにできた！
    * ただし、色が変。白黒にしかならない。要調査。

## 2021-10-28

* CanvasTestScene で UI で色を変えるた時にプレーンの色が白黒にしかならない件
    * unicolor.hlsl で float4 とする所を float としてしまっていた
    * 解決
## 2021-10-30

* enum の集合を保持できるクラス common::enum_set を実装
* CanvasTestScene でプレーンの色を UI から変更できるようになった。 UnicolorShader を追加。ShaderMixin によりシェーダーに独自のパラメータを持てるようになった。 ConstantBuffer 整理。

## 2021-10-31

* [c++]std::bitset と scoped enumでビットフラグ
    * https://qiita.com/cmaru/items/a6e9e0cc64ff920526c1
* [C++] 列挙体でビットフラグしたい
    * https://jumble-note.blogspot.com/2017/12/c.html

## 2021-11-02

* ShaderStage::VS | ShaderStage::PS で common::enum_set::operator | () を呼べないか検証
    * => namespace common の外で using すれば OK
        * だが、enum_set< N > の N をどうするか？といった問題もあるため、 { ShaderStage::VS, ShaderStage::PS } としてコンストラクタで受ける方法がベターか
* 