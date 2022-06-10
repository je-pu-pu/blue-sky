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

## 2021-11-01

* ShaderStage::VS | ShaderStage::PS で common::enum_set::operator | () を呼べないか検証
    * => namespace common の外で using すれば OK
        * だが、enum_set< N > の N をどうするか？といった問題もあるため、 { ShaderStage::VS, ShaderStage::PS } としてコンストラクタで受ける方法がベターか

## 2021-11-02

* CanvasTestScene の色を変えれるようになった
    * => 次は？
* パーティクルシステムを構築したい

## 2021-11-04

* set_sky_box() スクリプトコマンドを実行しても 2 回目以降 SkyBox が変更できない問題を発見
    * GraphicsManager::load_named_texture() がキャッシュされたテクスチャを返す事が原因
        * ひとまず GraphicsManager::set_sky_box() 内で古いテクスチャを削除するようにした

## 2021-11-05

* CanvasTestScene でポイントスプライトの加算合成を試した。良くある感じだが良い感じ。 脳で奏でる音楽 NextMind 版の演出に転用できそう。

## 2021-11-06

* 次何をするか？
    * Unity でポイントスプライトを表示してみる事を blue-sky よりも優先する
        * => Unity での実装結果を blue-sky に反映させられるため
## 2021-11-08

* Unity でのパーティクルシステム
    * https://docs.unity3d.com/ja/2021.2/Manual/class-ParticleSystem.html

## 2021-11-09

* Unity でパーティクルシステムのテストプロジェクトを作成

## 2021-11-10

* Unity でパーティクルシステムでは、指定の場所に粒子が集まってくるような演出はできないのではないか？
    * できる
        * https://liylia.hatenablog.com/entry/2017/11/07/184700

## 2021-11-11

* Unity でパーティクルシステムで、複雑な動きをしながら最終的に指定の場所に粒子が集まってくるような演出はできるか？
    * 要確認
    * https://www.youtube.com/watch?v=gdb9zvHh3r0

## 2021-11-12

* 脳で奏でる音楽 NextMind 版 Unity Project にパーティクルを組み込んだ。簡単に画面をリッチにできる事が分かった！

## 2021-11-13

* 脳で奏でる音楽 NextMind 版 Unity Project でパーティクルの大きさを曲に合わせて動的に変更する方法を覚えた！

## 2021-11-16

* Unity のパーティクルの GPU インスタンシングについて調査
    * ParticleSystem -> Renderer -> Render Mode を Mesh に、Enable Mesh GPU Instancing を On にする
    * https://docs.unity3d.com/ja/2018.4/Manual/PartSysInstancing.html
    * https://light11.hatenadiary.com/entry/2020/06/27/140603#GPU%E3%82%A4%E3%83%B3%E3%82%B9%E3%82%BF%E3%83%B3%E3%82%B7%E3%83%B3%E3%82%B0
    * https://light11.hatenadiary.com/entry/2020/06/27/194816

## 2021-11-17

* Unity のパーティクルシステムの CPU -> GPU 転送量について調査
    * マイフレーム以下の情報がパーティクル数分転送されているよう？
        * Position : POSITION.xyz
        * Color : INSTANCED0.xyzw
        * UI : TEXCOORD0.xy

## 2021-11-18

* Unity の ParticleSytem は CPU 描画であるとの記述を発見
    * https://nimushiki.com/2018/09/28/1203/

## 2021-11-19

* blue-sky パーティクルシステム設計
* パーティクル
    * 3D 空間座標
    * 各頂点の情報

* インターフェイス

```c++
void test()
{
    new ParticleSystem()
}
```

* Input Layout について
    * Getting Started with the Input-Assembler Stage
        * https://docs.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-input-assembler-stage-getting-started
    * Semantics
        * https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-semantics
        * https://docs.microsoft.com/ja-jp/windows/win32/direct3dhlsl/dx-graphics-hlsl-semantics

## 2021-11-21

* インターフェイス

```c++
void test()
{
    auto particle_system = new ParticleSystem();
    particle_system->set_max_particles( 1000 );
}
```

## 2021-11-24

* 可変引数テンプレート
    * https://cpprefjp.github.io/lang/cpp11/variadic_templates.html

* プログラミングの魔導書
    * http://longgate.co.jp/books/grimoire-vol1.html

* パーティクル・システム
    * https://ja.wikipedia.org/wiki/%E3%83%91%E3%83%BC%E3%83%86%E3%82%A3%E3%82%AF%E3%83%AB%E3%83%BB%E3%82%B7%E3%82%B9%E3%83%86%E3%83%A0

* O3DE
    * マルチプラットフォームのオープンソース3Dエンジン Open 3D Engine (O3DE) が発表
        * https://cginterest.com/2021/07/07/%E3%83%9E%E3%83%AB%E3%83%81%E3%83%97%E3%83%A9%E3%83%83%E3%83%88%E3%83%95%E3%82%A9%E3%83%BC%E3%83%A0%E3%81%AE%E3%82%AA%E3%83%BC%E3%83%97%E3%83%B3%E3%82%BD%E3%83%BC%E3%82%B93d%E3%82%A8%E3%83%B3%E3%82%B8/
* OpenMesh
    * https://blog.negativemind.com/2015/11/18/openmesh/    

* Input Layout について調べる

## 2021-11-25

* Effekseer をダウンロード、ツールを使ってみた。ソースコードも見てみた。
    * http://effekseer.github.io/jp/

## 2021-11-26

* Visual Studio 2022 をインストールした

## 2021-11-28

* 開発環境を Visual Studio 2022 にバージョンアップ

## 2021-11-30

* Visual Studio 2022 で AI によるコードブロックの入力は C++ では使えない事が判明
    * https://docs.microsoft.com/ja-jp/visualstudio/intellicode/intellicode-visual-studio

## 2021-12-03

* ホットリロードが有効にならなかったため、全てのプロジェクトで /INCREMENTAL, 非 /LTCG である事を確認した
    * しかし、/INCREMENTAL は /LTCG の指定によって無視されます。とのリンク時警告が表示されている
        * おそらく、ライブラリ側の設定も修正する必要がある

## 2021-12-04

* lua, libOVR を非 /LTCG で作成し、blue-sky にリンクしてみたが、前日のリンクエラーは消えず。

## 2021-12-06

* dumpbin というプログラムで .lib の情報を出力できる事が分かった
    * スタートメニュー > Visual Studio 2022 > Command Prompt のプロンプト内で実行できる
    * link というプログラムもある
* その .lib がコンパイルされた時のオプションが出力できるかはまだ未調査

* .lib のリンクを lib.cpp にまとめて記述して、順番にコメントアウトしていく事で、リンクエラーの特定に成功
* 主に lua.lib のコンパイルオプションの問題だったため、修正してリンク成功
* Visual Studio 2022 でエディットコンティニュー・ホットリロードは有効になった
    * しかし、C++ ゲームとの相性が悪いのか、実際にソースコードを編集すると、ほとんどの場合、エディットコンティニューはできない
* 一旦、エディットコンティニュー・ホットリロード・リンカの問題について考えるのは終了とした

# 2021-12-07

* TODO : パーティクル数固定、処理固定のパーティクルを実装してみる
    * 整理は後から

# 2021-12-08

* ParticleSystem の仮実装を開始
* ECS について調査
    * https://ja.wikipedia.org/wiki/%E3%82%A8%E3%83%B3%E3%83%86%E3%82%A3%E3%83%86%E3%82%A3%E3%83%BB%E3%82%B3%E3%83%B3%E3%83%9D%E3%83%BC%E3%83%8D%E3%83%B3%E3%83%88%E3%83%BB%E3%82%B7%E3%82%B9%E3%83%86%E3%83%A0
    * https://gist.github.com/LearnCocos2D/77f0ced228292676689f


* entity = class: no logic + no data OR at most small set of frequently used data (ie position)
* component = class: logic + data
* componentSystem = array of components of the same class

```
    foreach componentSystem in allComponentSystems do
        foreach component in componentSystem.components do
            component.update()
        end
    end
```

# 2021-12-13

* ParticleSystemTestScene を実装中

# 2021-12-16

* GraphicsManager::render_active_objects() が以下を呼んでいるのは変ではないか？
    * active_object->get_model()
    * active_object->render_mesh();
    * active_object->render_line();
    * ParticleSystem は Model を持っていない。描画するべき手描き風線も無いため、 render_line() も無意味。
    * 対応策
        * ParticleSystem に空の Model, 空の render_line() を持たせる
        * GraphicsManager::render_active_objects() を変える。再設計が必要。

* Entity Component System について調査
    * https://www.slideshare.net/DADA246/component-basedgameenginedesign-23304862
    * https://mikan-daisuki.hatenablog.com/entry/2015/10/22/220439

# 2021-12-17

* Game Programing Jems 6 : Game Object Component System を読む。

# 2021-12-26

* ParticleSystem 仮実装を進めた。
* GraphicsManager::render_active_objects() によって ParticleSystem が描画されるためには現状
    * Model, Mesh が必要

# 2021-12-27

* Game Programing Jems 6 : Game Object Component System 読了。

# 2021-12-28

* CEDEC2018 ECS 動画視聴
    * https://www.youtube.com/watch?v=T8lGD__aSm8

# 2021-12-29

* CEDEC2018 ECS 動画視聴完了
    * https://www.youtube.com/watch?v=T8lGD__aSm8

# 2021-12-30

* Unity 調査
    * Cube などの通常のオブジェクトは Mesh, Mesh Renderer ( , Box Collider ) コンポーネントを持つ
    * ParticleSystem は ParticleSystem コンポーネントのみを持つ ( Renderer は持たない )

# 2021-12-31

* ECS 動画視聴
    * https://www.youtube.com/watch?v=s6TMa33niJo

# 2022-01-01

* ECS 動画視聴
    * Entity Component System #1
        * 配信者は Sims 4 などのリード AI プログラマー "Rez Bot"
            * Twitter : https://twitter.com/rezibot
            * Web : https://bleachkitty.com/
        * https://www.youtube.com/watch?v=5KugyHKsXLQ
        * ECS とは
            * Component は機能を提供しない
            * System は状態を持たない
            * Entity は単に ID
        * 古い Component System では
            * O(G*C) の仮想関数呼び出しが起こってしまう ( G は GameObject の数, C はコンポーネントの数 )

# 2022-01-02

* ECS 動画視聴
    * Entity Component System #2
        * https://youtu.be/sOG4M-T__tQ
        * どう設計するべきか？を考察した動画
        * 検討内容
            1. Entity とは何か？
                * a. ID
                * b. index
                * c. class
            2. Component / System のクラス階層とは何か？
                * a. 単純なベースクラスと、複数のサブクラス ( 1 階層のみ )
                * b. 単純なベースクラスと、複数のサブクラス ( 複数階層 )
                * c. それぞれの System はユニーク
            3. System はどのように保存されるか？
                * a. ポインタの配列
                * b. 個々のオブジェクト
            4. Component はどのように保存されるか？
                * a. Entity に所有される
                * b. Manager に所有される
                * c. System に所有される
            5. どのように呼び出しコンテキストを生成し、また有効なコンポーネントを抽出するか？
            6. 共通コードをどのように管理するか？

        * 検討結果
            1. Entity とは何か？
                * a. ID ( おそらく )
                * Index は早いが、各 Entity が持っている Component はバラバラなため、 Index でアクセスする形式だとメモリが無駄になってしまう
            2. Component / System のクラス階層とは何か？
                * a. 単純なベースクラスと、複数のサブクラス ( 1 階層のみ )
                * System をループで更新したいし、必要に応じて System を追加・削除したいので c. はダメ
            3. System はどのように保存されるか？
                * a. ポインタの配列
                * 汎用性がなさそうなので b. はダメ
            4. Component はどのように保存されるか？
                * b. Manager に所有される
                * 同じ型の Component をメモリ上に連続して配置したいため

        
    * Entity Component System #3
        * https://youtu.be/4s7Py5nOg0E
        * #2 で考察した内容を実際に C++ コードで書いていく動画
    
    * Entity Component System #4
        * https://youtu.be/4s7Py5nOg0E
        * #3 に引き続き、 C++ コードで書いていく動画
        * BreachUtils というライブラリを使っていたが検索しても見つけられなかった。配信者のオリジナルライブラリか？
            * BreachUtils は new, delete のオーバーロードに使われていた。
        * Component がメモリ上に並ぶよう、new をオーバーライドする。

# 2022-01-03

* ECS 動画視聴
    * Entity Component System #5
        * https://youtu.be/vm7kuSIh6oE
        * コンパイル時に ID を振る仕組みが上手く動かない
            * 代替案 #1 enum class で ID を定義する
                * enum class は拡張できないため、ゲームエンジン側で定義したコンポーネント ID をゲームプレイ側で自由に追加する事ができないのでダメ
            * 代替案 #2 enum class で ID を定義し、最後の項目に MAX のような値を定義しておく。ゲームプレイ側で enum class を定義し、 kMAX から始める。
                * 動くが満足していない
                * 利点 : 速い
                * 欠点 : 管理が難しい
            * 代替案 #3 GUID を使う
                * Visual Studio > メニュー > ツール > GUID の生成で生成できる！！！
                * GUID をハッシュマップに使うとパフォーマンスはどうか？
                * 利点 : 速い
                * 欠点 : 手作業で GUID を生成しなければならない
            * 代替案 #4 ファイルパスをハッシュして ID として使う
                * 環境が異なるとパスも異なるため、一貫性が失われる問題
                * コンパイル時文字列ハッシュについては Game Engine Gems 3 に載っている
                * 利点 : ほとんど自動
                * 利点 : 実行時に ID を計算可能 ( たとえば Lua のようなスクリプトでも )
                * 欠点 : 多くのコンポーネントがあった場合、コンパイルが遅い

# 2022-01-04

* ECS 動画視聴
    * Entity Component System #6
        * https://youtu.be/39e1qpsutBU
        * System の実装
            * 以下のように、ComponentSystem のテンプレート引数に、System が使う Component を複数指定し継承する
                * class RenderSystem : public ComponentSystem< Renderable, TransformComponent >

# 2022-01-06

* ECS 動画視聴
    * Entity Component System #6
        * System が使用するコンポーネントの一覧 Comps を、テンプレートパラメーターパックを使って展開し、Entity の持っている Component と比較するテクニックが紹介された。
        * https://cpprefjp.github.io/lang/cpp11/variadic_templates.html

# 2022-01-07

* ECS テスト実装を開始
    * Rez Bot の ECS 動画を参考に、blue-sky に ECS のテスト導入を開始
    * ひとまず GameObject に add_component() と get_component() を実装。

# 2022-01-08

* ECS 動画視聴
    * CppCon 2015: Vittorio Romeo “Implementation of a component-based entity system in modern C++”
        * https://youtu.be/NTWSeQtHZ9M
        * コンパイル時に多くを解決するので実行速度が速いが実装が複雑
        * ゲームエンジンとゲーム本体を分離する事は考慮されていなさそう
    * Vittorio Romeo: Implementation of a multithreaded compile-time ECS in C++14
        * https://youtu.be/3N1pLtTV2Uc
        * lambda 式が使われている
        * マルチスレッディングが考慮されている
        * さらに複雑

# 2022-01-10

* ECS テスト実装の続き
    * 以下の枠組みを実装してコンパイルできた
        * エンティティの追加
        * エンティティへのコンポーネントの追加
        * システムの追加
        * システムの実行

# 2022-01-11

* ECS テスト実装の続き
    * Entity Component System #6 の動画を参考に、System のテンプレートパラメータで System が使用する Component を複数設定可能に。
        * https://youtu.be/39e1qpsutBU

# 2022-01-14

* 他人の作った ECS の調査
    * CORGI
        * http://google.github.io/corgi/index.html
        * CORGI = コーギー ( 犬 )
            * https://www.google.com/search?q=CORGI&tbm=isch
        * Google の公開している ECS のライブラリ
        * System はなく、ComponentData がデータ、それを継承した ( ？ ) Component がロジックを含むという仕組みらしい

# 2022-01-16

* CORGI 調査
    * Entity を追加すると EntitiManager::entities_ に追加される
    * entities_ は VectorPool<Entity>
    * VectorPool の各要素 ( VectorPoolElement ) は以下のメンバ変数を持つ
        * data ( Entity )
        * prev ( 前の要素の index )
        * next ( 次の要素の index )
        * unique_id ( ID )
    * Entity のメンバ変数は uint16_t + bool のみ ( ID と削除するべきかのフラグ？ )

# 2022-01-17

* CORGI 調査
    * Entity クラスは以下のメンバ変数を持つ
        * 
    * EntityManager クラスは以下のメンバ変数を持つ
        * VectorPool<Entity> entities_;
        * std::vector<ComponentInterface*> components_;
    * Component クラスは以下のメンバ変数を持つ
        * VectorPool<ComponentData> component_data_;
        * std::unordered_map<EntityIdType, ComponentIndex> component_index_lookup_;
    * ComponentData 構造体は以下のメンバ変数を持つ
        * EntityRef entity                      ( Entity への参照 )
        * T data;                               ( 各コンポーネントのデータ )
    * EntityRef ( = VectorPool< Entity >::VectorPoolReference ) は以下のメンバ変数を持つ
        * VectorPool< Entity >* container_;     ( Entity の配列へのポインタ )
        * size_t index_;                        ( Entity の配列上でのインデックス)
        * UniqueIdType unique_id_;              ( Entity の ID )
    * `entity_manager.RegisterComponent< CounterType >( & component );` でコンポーネントを登録する
        * EntityManager::RegisterComponent() は components_ にコンポーネントを追加する
    * `entity_manager.AddEntityToComponent< ComponentType >( entity );` でコンポーネントを Enitty で使用する
        * EntityManager::AddEntityToComponent() からは `component->AddEntityGenerically( entity );` が呼ばれる

# 2022-01-18

* CORGI 評価
    * ComponentData から Entity の参照は高速
        * entity.container_[ entity.index ]
    * Entity から Component の参照はやや高速
        * EntityManager::GetComponentDataAsVoid()
            * components_[ component_id ]                               ( Vector の index 参照 )
        * Component::GetComponentDataAsVoid() -> GetComponentData()
            * Component::GetComponentDataIndex()
                * component_index_lookup_.find(entity->entity_id());    ( unordered_map の検索 )
            * component_data_.GetElementData(data_index);               ( Vector の index 参照 )
    * Entity のメモリ配列
        * 各要素は Entity
            * EntityIdType  entity_id_;                 ( = uint16_t )
            * bool          marked_for_deletion_;
    * Component のメモリ配列
        * 各要素は ComponentData
            * EntitiRef
                * VectorPool< Entity >* container_;     ( Entity の配列へのポインタ )
                * size_t index_;                        ( Entity の配列上でのインデックス)
                * UniqueIdType unique_id_;              ( Entity の ID )
            * data                                      ( 実際のデータ )

# 2022-01-19

* ECS に関する記事調査
    * https://namekdev.net/2017/03/why-entity-component-systems-matter/
        * ECS とは何か？
        * 理由 : より簡単なリファクタリング
        * クラス階層は悪
        * ゲーム開発だけ？
        * 長所は分かったけど短所は？
        * MMO の開発に有効との別記事あり
            * http://t-machine.org/index.php/2007/09/03/entity-systems-are-the-future-of-mmog-development-part-1/

# 2022-01-20

* ECS に関する記事調査
    * http://t-machine.org/index.php/2007/09/03/entity-systems-are-the-future-of-mmog-development-part-1/
        * Operation チームの開発者
        *  Scott Bilas ( 2002 に GDC で ECS について発表したダンジョンシージのプログラマ？ ) と連絡をとった結果、ブログを進められた
            * https://gist.github.com/LearnCocos2D/77f0ced228292676689f
        * 導入のみ。MMO のように複雑で、運用が開始されてからもコードを変更するようなプロジェクトでは、ECS しかないじゃないか？みたいな感じ

# 2022-01-21

* ECS に関する記事調査
    * Entity System とは何か？
        * http://t-machine.org/index.php/2007/11/11/entity-systems-are-the-future-of-mmog-development-part-2/
        * あるレイヤーにおいては、オブジェクト指向か Entity System かは排他的であるため、どちらかを選ばなければならない

# 2022-01-22

* ECS に関する記事調査
    * Entity System とは何か？
        * http://t-machine.org/index.php/2007/11/11/entity-systems-are-the-future-of-mmog-development-part-2/
        * Entity とは何か？
            * ゲームの世界で認識できるすべての「もの」に対して 1 つの Entity がある
            * ゲーム中に戦車が 1,000 体出るなら、Entity も 1,000 個ある
            * Entity にはデータもメソッドもない
        * Component とは何か？
            * ゲーム中のアイテムの、ひとつの側面
            * アイテムの Entity がアイテムの色々な側面を表すには、側面ごとに 1 つのコンポーネントを持つ
            * ひとつの Component は、ひとつの重要なことを行う
            * 補足 : オブジェクト指向プログラミングでは、この複数の側面の概念がない。複数の側面が一つの塊としてハードコードされる
                * C++ の多重継承、 Java のインターフェイスによって一部似たような事はできるが、すぐに破綻する
# 2022-01-23

* ECS に関する記事調査
    * Entity System とは何か？
        * http://t-machine.org/index.php/2007/11/11/entity-systems-are-the-future-of-mmog-development-part-2/
        * System ( または subsystem ) とは何か？
            * Enitity System ではデータを Entity + Component に、コードを System に分割する
                * オブジェクト指向プログラミングでは全てをオブジェクトにする
            * 各システムは、 ( 各システムが独自のプライベートスレッドで動いているように ) 継続的に実行される
            * ゲームにおける一般的な System は以下のようなもの
                * レンダリング System
                * アニメーション System
                * 入力 System

# 2022-01-24

* ECS に関する記事調査
    * Entity System とは何か？
        * http://t-machine.org/index.php/2007/11/11/entity-systems-are-the-future-of-mmog-development-part-2/
        * System ( または subsystem ) とは何か？
            * レンダリング System
                * 16 ミリ秒ごとに立ち上がり、レンダリング可能な Component を持つ全ての Entity をレンダリングし、スリープする
            * アニメーション System
                * 新しいアニメーションをトリガーしたり、( 例えばプレイヤーが移動の途中に方向を変える等、 ) 既存のアニメーションを変更したりするものを探し、影響を受ける各 Entity のデータを更新する
            * 入力 System
                * ゲームパッドやマウスなどの入力をチェックし、プレイヤーによってコントロールされているとマークされている入力可能な Component を持っているエンティティーの状態を変更する
        * オブジェクト指向プログラミングでは、戦場に 100 のユニットがあった場合、論理的には、ユニットが呼び出す事のできるメソッドのコピーが 100 個ある
            * 実際には、これらのメソッドは裏で共有され、メモリの無駄遣いを回避している
            * スクリプト言語の場合、メソッドの共有を行わない事もあり、すべてのメソッドをインスタンスごとに個別に変更できる
        * 一方、Entity System では、戦場に 100 のユニットがあり、それが Entity で表されている場合、Entity にはメソッドが含まれないので、ユニットで呼び出すことのできる各メソッドのコピーは 0
            * Component にもメソッドも含まれていない
            * 代わりに、各側面に外部の System があり、System には、その System と互換性のあることを示す Component を所有する Entity で呼び出す事のできる全てのメソッドが含まれている

# 2022-01-26

ECS に関する記事調査

Entity System とは何か？

http://t-machine.org/index.php/2007/11/11/entity-systems-are-the-future-of-mmog-development-part-2/

## それは Entity System ではない！

少なくとも、私が一緒に仕事をしてきたほとんどの人は、これをEntity Systemとみなしています。
しかし、1つのプロジェクトの中でも、ESとは何かということについて、複数の異なる考え方があり、互換性のあるものもあれば、互換性のないものもあります。以下は、私が出会った、人々がESと考えた他のアイデアの一部です。

エンティティシステムの代替定義 :

1. ESは、OOPとは異なるクラス継承のモデル、特にフィールドの継承を提供します。あるエンティティクラスに対して、そのクラスが持つアスペクトを宣言し、実際のクラスは実行時に異なるアスペクトをすべてつなぎ合わせて検索されます。
2. 1.と同じですが、文字通りフィールドだけでなくメソッドにも適用されます。つまり、関数/メソッドの継承をミックスマッチさせる面白い方法を提供します。メソッドが操作するフィールドに付属している限り、あるいは「アスペクトXを持つとアスペクトYも自動的に持つ」ような「前提条件」システムが実装されている限り、これは非常にスムーズに機能する。注：これは「モジュラー・オブジェクト」の実装である。「アスペクト」は、OOPの標準的な理論に基づく完全なオブジェクトである。
3. 1.または2.の変形で、アスペクトデータはコンパイル時に多数のOOPクラスにあらかじめコンパイルされており、実行時には「通常の」クラスを実行しているだけで、動的なルックアップのコストは発生しない。「アスペクトXを持つというエンティティEがあります。誰かが関数Zを呼び出そうとしたので、一体どうすればいいのか、関数表を作るのでちょっと待ってください...」
4. 標準的なOOPクラスをランタイムデータにコンパイルする方法（注：OOPらしさから切り離す）。特に、バイナリ＋データのチャンクではなく、ストリームデータとして扱えるようにします。これの主な価値は、コードキャッシュに利用可能なメモリがごくわずかで、追加のコードをフェッチするためのアクセスが非常に遅く、CPUを介してシーケンシャルデータをストリームするための帯域幅が非常に広いハードウェアとうまく連携することです。典型的なPCのアプローチとは対照的に、PlayStationコンソールは特にこのアプローチを好みます。この場合、ゲームプログラマーとゲームデザイナーは、エンティティシステムが存在することに気づかないということに注意してください - これはランタイム/コンパイル時のパフォーマンスの最適化であり、デザインの特徴ではありません。
5. Observerパターンの過剰な使用を中心に展開するシステム設計だが、標準的なOOP技術でそれを実現している。ESの概念をよく使うが、利点があまりないため、このような例を挙げました。何を隠そう......私が初めてESを知った時も、そんな状況だったのです。だから、個人的には、これをESにカウントしないのが本音です。これらは、ESではありません。が、「オーマイガット！ESが欲しい！」が、自分は真のESだと思い（主張し）続ける人が多い。
    * 注意:個人的には、Entity System の使い方で「これは本当にやってはいけない」クラスの1つだと考えています。
        なぜなら、独立したObserverメソッドを大量に実装した巨大なOOPクラスを作成することになってしまうからです。それは一般的なアプローチとしては問題ありませんが、ESが提供するもの、特に柔軟性と複合継承の領域をほとんど失ってしまうのです。

## ES を気にする人は誰？そして、なぜ？ ( これはどこから発明されたの？ )

1. グラフィック/レンダリングエンジンを書き、それをコンピュータゲームの他の部分から分離/独立させようとすることに慣れているコーダー。
彼らは、ゲーム世界に存在するすべてのOOPオブジェクトの部分的な情報（2つか3つ以下の側面）だけを使用して、グラフィックスエンジン全体を書くことができることを学びました。
彼らは以下を使います
    * Renderable
        * 毎フレーム、画面に描画する必要があるのか？[その描画に必要なメソッドをすべて汎用的に備えています
    * Updateable 
        * ゲームループのチックに反応して変化する可能性があるか？[アニメーションはその好例で、レンダリング回数とは無関係に内部状態が変化する]。

# 2022-01-28

ECS に関する記事調査

Entity System とは何か？

http://t-machine.org/index.php/2007/11/11/entity-systems-are-the-future-of-mmog-development-part-2/

## ES を気にする人は誰？そして、なぜ？ ( これはどこから発明されたの？ )

2. 粗粒度マルチスレッディングを可能にするためにアスペクトの概念自体に注目するマルチスレッドコーダー ( 例 : Updateable ではなく Renderable )
ひとつの「側面」につきひとつの「スレッド」を使用する事ができます。そしてこれは安全であることが保証されています。「側面」は定義上、互いに独立しているからです。
一般的な Entity System は十数個以上の「側面」を持つため、最大十数個のCPUコアの効率的なパフォーマンスアップをタダで手に入れた事になります。

そして、難しさとパフォーマンスの向上の次のレベルに行くと、 ( 文字通り、分割できないもの ) を探します。
デフォルトの立場では、手動でハードコードされたアトミック操作だけがカウントされ、OOPオブジェクトは確かにあらゆる種類のマルチスレッドの個別ピースでいっぱいなので、粗すぎるのですが、もしかしたらエンティティ内のコンポーネントはアトミック性の単位とするには十分小さいかもしれません。

3. 「あらゆるものが何にでもなる」事を可能にしたいゲームデザイナー
カメラは人を撃ちますか？弾はプレイヤーからの入力を受け付けますか？いいえ？
Unreal Tournament はプレイしましたか？もし、あなたがRedeemerの代替発射モード（ヒント：弾頭内部の一人称視点で、手動でロケットを「ワイヤーで飛ばす」ことができる）を持っているが、先の質問で「はい」と答えなかった場合は、誰かを見つけて見せてもらう必要があります。

# 2022-01-29

ECS に関する記事調査

Entity System とは何か？

http://t-machine.org/index.php/2007/11/11/entity-systems-are-the-future-of-mmog-development-part-2/

3. 

ESは、あらゆるものを、あらゆるものとして、交換可能に、再コード化なしで使用できるようにします。これはデザイナーにとって素晴らしいことです。これまでコーダーは、自分自身のクソコードを書くために、それなりのパフォーマンスとバグのないものにするために、何らかの制約を加えなければなりませんでしたが、デザインに対する人工的な制約がすべて突然取り払われるのです。

少なくともEntity Systemsでは、安全なコードを強く型付けしながらも、どんなメソッドにもどんなオブジェクトでも自由に渡すことができるのです。

4. Observerパターンでちょっとやりすぎなコーダー。

もし、あなたがこの件に関して確信が持てず、Observersをよく使っているのなら、自分にこう問いかけてみてください。「1つのベースクラスだけでは十分ではない」と感じたことはありませんか？

例えば、いくつかの異なる Observers を実装した基本クラスはうまく機能しましたが、システム/ゲーム/アプリケーションが拡大するにつれ、その基本クラスにもっと多様性が必要になり、複数の異なるバージョンが必要になり、それらの異なるバージョンの *あらゆる可能な組み合わせ* が必要になったことがあります。

## 今日の考え

エンティティシステムでうまくプログラミングすることは、リレーショナルデータベースでプログラミングすることに非常に近いです。

ESを「リレーション指向プログラミング」の一形態と呼ぶのは無理からぬことだろう。

これはフォローアップの投稿が必要ですが、あなたはそれについて考えたいかもしれません :) 上記のステートメントがどのように真になり得るかという点（ヒント：各システムのためにあなたが書くコードのほとんどがどのように見えるか、そしてそれが正確に何をしているかを考えてみてください）と、それが真である場合のノックオン効果という点から、両方について考えてみてください。

RDBMS と OOP の間の変換コストは、コーディング時と実行時の両方で非常に高く、MMO を作成する際の最大の開発問題の 1 つとなっていることを念頭に置いてください。

# 2022-01-29

ECS に関する記事調査

Entity Systems are the future of MMOG development – part 3

http://t-machine.org/index.php/2007/12/22/entity-systems-are-the-future-of-mmog-development-part-3/

## 質問

まず、最初の2つの投稿に対して、良い質問がたくさん出ていますね。

* エンティティのOOP実装には、どのようなデータとメソッドが格納されているのでしょうか？
* データはどこに "住んで "いるのか？
* オブジェクトの初期化はどのように行うのですか？
* AOPフレームワークでは実現できない、ESのもたらすものは何でしょうか？
* エンティティシステムとSQL/Relational Databasesの関連は？(前回に引き続き、私自身の質問です。）
* Entity とはいったい何なのか？

まずは最後のひとつから。

## エンティティとは、いったい何なのでしょうか？

もちろん、コンセプトの観点からは前回の記事ですでに取り上げており、次のように定義しています。

> ゲームワールド内の識別可能なものには、1つのEntityがあります。エンティティはデータもメソッドも持っていません。

素晴らしい。でも何人かの人が 疑問に思っていたのですが 実装を始めるとき 実際にそれは何なのでしょうか？それはクラスなのでしょうか？データの配列なのか？Componentのリストなのか？

エンティティはクラスのスタブでもなく、データの構造体でもなく、理想的にはComponentのリストでもないのです。エンティティは単なる名前なのです。前回の記事で、エンティティは「すべてのゲームオブジェクトを別のアイテムとしてタグ付けする以上のことはしない」とも書きましたが、ここが重要な点で、エンティティはゲーム内のオブジェクトに対する一意のラベルに過ぎません。

ちょっと具体性に欠けましたね。なぜなら、多くの人がそのようなエンティティを実装しており、何が正しいか間違っているかは誰にもわからないからです。

しかし...実は、これは*間違った*方法だと思います。このエンティティシステムの記事の最後を読む頃には、おそらくあなたも私に同意してくれるでしょう ;)しかし、論理的な実装としてではなく、パフォーマンスの最適化として（例えば、システムの他の部分からその詳細を隠すことによって）そうするのであれば、長期的にはコードに問題を起こすことは少なくなるはずです。

そこで、質問に直接お答えします。

> エンティティとは何ですか？

> エンティティは番号（世界で一意の番号）である

もちろん、文字列やその他のユニークなラベルは、コンピュータの実装方法だけでなく、数学者の考え方からしても、単なる数字の一種であることを忘れてはならない。例えば、文字列の中に簡単にツリーをエンコードすることができます（例えば、C言語から派生したほとんどのOOP言語でクラスやメソッドなどの名前空間をナビゲートするために使われている「root.node1.leaf」）もちろん、数字でもそのようにエンコードできます、例えば「123045607」（ドットに0を使って、つまり「123.456.7」となります）。でも......これは本当にやめておいたほうがいいですよ。

その1：階層的な名前をつけたがるのは、OOPプログラマーだけ。よく考えてみると、これはかなり馬鹿げた考えです。通常、私たちが考えるのに慣れているOOPの方法を手放すことを暗黙のうちに拒否しているのですが、OOPの方法はまさに、ESを使うことで解決しようとしている問題の大半を引き起こしているのです。

だから、階層的なエンコーディングに誘惑されないでください。また、エンティティ名には一切エンコーディングをしないでください。エンティティ用のメタデータを作成するには、もっと良い場所があります（私を信じてください）。

そして、エンティティの実装を、これからはGUID（"Globally Unique IDentifier"）と呼ぶことにします。

# 2022-01-31

ECS に関する記事調査

Entity Systems are the future of MMOG development – part 3

http://t-machine.org/index.php/2007/12/22/entity-systems-are-the-future-of-mmog-development-part-3/

## エンティティのOOP実装には、どのようなデータとメソッドが格納されていますか？

前の質問に対する答えから、この質問に対する答えは簡単です：「ない」です。エンティティは単なるラベルであり、そのラベルに関連するメタデータを保存したり操作したりすることはできますが（例えば、現在メモリ内にあるすべてのエンティティ名のリストを持っていて、すべてを壊すことなく名前を変更できると便利です）、GUID（文字列、数値、何でも！）以上または以下として考えるべきものではありません。

# 2022-02-01

ECS に関する記事調査

Entity Systems are the future of MMOG development – part 3

http://t-machine.org/index.php/2007/12/22/entity-systems-are-the-future-of-mmog-development-part-3/

## データはどこに "住んで "いるのか？

最初にESを作ったときは、個々のEntityをクラスとして実装し、そのクラスにはメソッドを入れませんでしたが（それでは標準的なOOPになってしまいます！）、データはその中に入れました。データの保存場所を気にする必要があるとは思いもよらなかったので、自然な場所、つまりEntityの中に保存しておきました。

もちろん、私は重大な間違いを犯していた。なぜなら、OOPプログラマーにとって「当たり前」と感じることは、ESプログラミングにおいては「完全に間違っている」ことがほとんどだからだ。

とにかく、前回ESを作った時はもっと賢くなっていたので、Componentsの中にデータを入れました。全部です。

ガッチャ2：すべてのデータがComponentsに入る。すべてです。例えば、ゲーム内のオブジェクトのX/Y/Z座標のような「本当に一般的な」データを、エンティティ自体に入れることができると思いますか？いいえ。そこに行ってはいけません。データをEntityに移行し始めた時点で、失敗なのです。定義によると、データの有効な場所はComponentの中だけです。

その仕組みは？

さて、ここからはもう少し慎重にコンポーネントを考える必要があります。以前、私はこう言いました。

コンポーネントは、エンティティがこの特定のアスペクトを所有しているとラベル付けします。

つまり、繰り返しになりますが、ComponentはEntityそのものと同じように、単なる「ラベル」のひとつに過ぎないのです。そうでしょう？違うんです。前回の投稿では、シンプルにしようと思って、かなり多くのことを省略してしまいました。一番最初に作ったESでは、コンポーネントを単なるラベルとして扱ったので、その説明が「シンプル版」として最も分かりやすく出てきてしまったのです。もっと詳しく言うと

Componentは、Entityに特定のアスペクトを提供するものの塊である。

...ここで、「提供する」というのは、「これを動作させるために必要なすべてのことを行う」という意味です。つまり、すべてのComponentは、標準的なOOPオブジェクトとして実装することができるのです。Entity を定義するには、名前（GUID）と必要な Component クラスのリストを提供し、Entity をインスタンス化するには、各クラスから 1 つのオブジェクトをインスタンス化するだけです（そして、これらのインメモリ オブジェクトを GUID に何らかの形で関連付ける必要がありますが、おそらく Entity を GUID とコンポーネントインスタンスのリストを含む空のクラスとして実装します（これは避けるように言ったものの、また後で戻ってきます））。

しかし、もうお気づきかもしれませんが、私はES実装の内部でOOPを使うことには断固として反対です。これは、OOPそのものを嫌っているわけではなく、私のES開発の経験上、経験豊富なOOPプログラマーが、不適切なところにOOPを忍び込ませようとし、それが良いアイデアだと錯覚してしまうことがあるからです。さらに悪いことに、私が過去に所属していたチームでは、OOPプログラミングが得意であればあるほど、これに抵抗するのは難しいようです...。

コンポーネントをOOPオブジェクトとして実装することは可能です。しかし、前回の記事で紹介した「システム」の定義に戻ると、コンポーネントに作用するメソッドはすべてコンポーネントではなく、システムの中にあるべきことがわかります。実際、前回の投稿を読み返してみると、私は、システムとはまさにこのようなものであると明確に説明していることに気づきました。「システムは本質的にコンポーネントのためのメソッド実装を提供する」のです。

ゲッターとセッターは本当に便利なものです。もし、ESをOOP言語で実装しているのであれば、Componentをオブジェクトとして実装し、get/setのメリットを享受することができます。しかし、もしそうするならば、フライウェイトオブジェクトとして実装しなければなりません（フライウェイトパターンを知らなければ、ググってみてください）。
https://www.google.com/search?q=flyweight+pattern

データはどこに "住んで "いるのか？

各Componentは、単に分類ラベル（例：「Renderable」）と、その目的に関連するデータの構造体／配列／リスト／その他であり、すべてのデータはComponentの中に存在します。

# 2022-02-02

ECS に関する記事調査

Entity Systems are the future of MMOG development – part 3

http://t-machine.org/index.php/2007/12/22/entity-systems-are-the-future-of-mmog-development-part-3/

## ゲームオブジェクトの初期化はどのように行うのですか？

オブジェクトやエンティティの初期化について、これまで何も触れてきませんでした。オブジェクト指向が初期化に関していかに奇妙なものであるかを理解したのは、エンティティシステムを使おうとしてからです。オブジェクト指向の主な説明とは無関係な「魔法のメソッド」（コンストラクタ、デストラクタ、ファイナライザなど）がありますが、代わりに、オブジェクト指向システムを稼働させ、それを維持し、最後にすべてを取り壊すためのメタプログラミングの一部になっているのです。それまでは、それらのメソッドがいかに場違いなものであるかに気づかなかった...。

OOPの人たちには脱帽です。彼らはメタプログラミングをOOPにきちんと統合しているので、ほとんどの言語では気づかれることはないでしょう。しかし、私はまだESで同じトリックを見たことがありません。ですから、初期化が少し...「違う」...という事実に慣れてください。

本当にいろいろな問題があるんです。

* アーキタイプとは何ですか？
* エンティティのアーキタイプはどのように定義しているのですか？
* 1つのアーキタイプから複数の新しいエンティティをインスタンス化する方法は？
* インメモリエンティティをどのように保存し、後で再インスタンス化できるようにするか？

## エンティティのアーキタイプを専門用語で何と言いますか？

ないようです。本当に。それぞれの背景の影響を受けて、いくつかの言葉が飛び交っていますが、同じような意味で使われているのをよく耳にします。

私のお気に入りは

* テンプレート
* モデル
* アセンブリー

(最後の言葉は、実は類語辞典で「クラス」に相当するものを探して選びました ;）。

「テンプレート」と「モデル」の問題は、どちらもプログラミングで非常に頻繁に使われる一般的な用語であることです。
エンティティは、他のものにはほとんど使われないので、用語としてはかなり優れています（そして、主な明らかな競合相手は、現在、ゲームプログラマによって広く「ゲームオブジェクト」と呼ばれています）。
私は過去に両方を使ったことがあります。互換性がある。(キャッキャッ!)

では、"Assemblage "の方は？

エンティティのアーキタイプはどのように定義するのですか？
1つのアーキタイプから複数の新しいエンティティをどのようにインスタンス化するか？
メモリ内のエンティティを保存して、後で再インスタンスできるようにするには？

大きなテーマです。本当に大きいです。ここで説明するには大きすぎます（ああ、これで次の投稿の内容がわかりました...）。

# 2022-02-04

http://t-machine.org/index.php/2007/12/22/entity-systems-are-the-future-of-mmog-development-part-3/

## AOPフレームワークでは実現できない、ESのもたらすものは何でしょうか？

その答えは、「ほとんどすべて」です。

AOP (Aspect-Oriented Programming) は、Entity System が解決する問題の解決にはあまり役立ちませんし、ES から得られる新しい機能 (たとえば、メモリやデータ性能の改善) も提供しません。

ESは、OOPが絡め取ったような、それぞれが独立した多くの事柄が進行しているシステムを取り上げ、それらを分離し、独立したものとして生かすものです。また、プログラムの異なる側面間の簡単な相互運用を非常に効率的に管理し、それらを自由に切断、再接続できるようにします。

AOPは、基本的に依存関係にあるものが一緒に進行しているシステムを、OOPが絡め取って、「別々に、しかし一緒に」推論できるようにします。定義上、Entityの異なる側面（Componentとして登録されている）は非依存であり、この「一緒に」というのは関係ないことである。

しかし......理想的にはESで行うべきことをOOPで実装し、それに新しい機能を追加したい場合、AOPは確かに役立ちます。なぜなら、絡まったOOPシステムは、誤ってものを壊さずに修正するのが面倒になり、AOPは変更の影響をより正確にフォーカスするのに役立ちます。しかし、AOPは、最初に望んだESを実装することの代用品としては不十分です。

また、「エンティティシステムでできること」と「アスペクト指向プログラミングでできること」の間には、かなり広い範囲でクロスオーバーがあります。これは主に、どちらも基本的に関数-ディスパッチレベルでデータドリブンであるためです（これについては後で説明します）。AOPはプログラムのコードに反応し、それを取り囲むように使用しますが、ESはプログラムのデータに反応するように使用します。

また、「エンティティシステムでできること」と「アスペクト指向プログラミングでできること」の間には、かなり広い範囲でクロスオーバーがあります。これは主に、どちらも基本的に関数-ディスパッチレベルでデータドリブンであるためです（これについては後で説明します）。AOPはプログラムのコードに反応し、それを取り囲むように使用しますが、ESはプログラムのデータに反応するように使用します。

この2つを同時に使うのは、とてもエキサイティングなことだと思うのですが、私自身はまだそれを試す勇気がありません。

# 2022-02-06

http://t-machine.org/index.php/2007/12/22/entity-systems-are-the-future-of-mmog-development-part-3/

## エンティティシステムとSQL/Relational Databasesの関連は？

もうお分かりかもしれませんが、この記事の冒頭から、上記の回答のほとんどがこのテーマなのです。

数学的に言えば、エンティティはデータベースのキーであり、RDBMSで見られるようなものである。

同様に、純粋に抽象化すれば、「Entity #7742 を構成するコンポーネント・インスタンスの集合」は、文字通り、データベースの Query となる。

だから、理想的には、各エンティティの構造体/クラス表現内にコンポーネント・インスタンスをリストとして格納したくないと最初に言ったのです。基本的に、このセットはリストとして定義されておらず（リストは静的なもので、あなたが変更しなくてもメンバーは変わりません）、クエリとして定義されています（そのメンバーは、あなたが望むと望まないとにかかわらず、常に変化しています）。

もちろん、リストを単純なクエリとして使うこともできるが（そのクエリとは「現在このリストにあるものは何か」というもの）、これはかなり貧弱で柔軟性に欠けるクエリである。クエリの動的な性質を利用し、必要なときにクエリを実行してコンポーネント（つまりデータのVALU）を取得すれば、人生はもっと面白くなる。

# 2022-02-07

ゲームは巨大なデータベースであり、常に動的なクエリを実行しているのだ、ということです。現在のパフォーマンスは、高速なRDBMSであっても、家庭用PCで1フレームあたり数千のクエリを実行するには遅すぎるのです。

しかし、これらの記事は、通常のゲーム開発ではなく、多人数参加型オンラインゲーム開発のためのESに関するものであり、それによって特に2つのことが変わってきます。

1.ゲームのほとんどは家庭用PCで動作しているのではなく、すでに何十億ものQLクエリ（最近はほとんどSQLですが、MS SQL Server、Oracle、MySQLのいずれか）を実行している、任意に強化したサーバーファームで動作しているのです。

2.レンダリング速度よりもデータの方がはるかに重要です。3Dで描かれた風景が驚くほど美しくなることはほとんど問題ありませんが、その風景があたかも実在するかのように振る舞い、反応するためのデータ管理（ワールドロジック、ゲームロジックなど）は、今でもかなり困難だと感じています。

## 今日一日、新たな思いで...。

MMOのサーバーファームにESを展開し、エンティティやコンポーネントの集計をSQLクエリとして保存するだけで、どんなことができるかを考えてみてください。手始めにいくつか紹介しましょう。

1."リレーショナル vs OOP "の苦しみから解放される

MMOサーバーのパフォーマンスで最も遅い部分の1つは、OOPオブジェクトをRDBMSのテーブルに変換してデータベースに保存できるようにし、ゲームがそれを読み取る必要があるときに再び変換することです。また、シリアライゼーション処理を行うために、退屈で煩わしく、保守が厄介なコードに大量のプログラマーの時間を浪費しています。

2.無償で並列化を大量に実現

すべての処理をデータドリブンにし、同期を取らなければならないデータ構造がなければ、実行性能を上げるために「CPUを増設する」ことはとても簡単なのですが......。

3.データ構造の代わりにSELECTを使えるのは他にどこがありますか？

各システムが処理を行う際、おそらく「私のコンポーネントを持つすべてのエンティティ」を繰り返し処理することになるでしょう。これは、標準的なデータ構造に簡単に格納できるものなのでしょうか？それとも、QLクエリにした方がいいのでしょうか？また、この方法で処理を始めると、他にどんなことが実現できるのでしょうか？(ヒント: ここには低レベルのネットワーク・プログラミングのための素晴らしい利点があります...)

# 2022-02-08

Entity Systems are the Future of MMOs Part 4

http://t-machine.org/index.php/2008/03/13/entity-systems-are-the-future-of-mmos-part-4/

多人数参加型エンティティシステム。イントロダクション

さて、過去3回の記事のタイトルでちらつかせていた、ESとMMOの関係とは？(まだ読んでいない方はこちらからどうぞ。）

簡単に言うと、「データがすべて」なのです。そして、データというのは、システムの消費者（ゲームクライアントなど、従来のゲームにあるMMOの唯一の部分）ではなく、システム全体（MMOなど）を扱わなければならないときは、ほとんどの人が考えるよりずっと難しいものなのです。

もちろん、それよりももっと詳しく見る必要があります。まず、背景を説明しますと...

# 2022-02-09

## 多人数参加型ゲーム開発入門

MMOの開発では、いくつか気をつけなければならないポイントがあります。プロのゲーム開発者の多くはすでにこれらの一部または全部を知っていますが、私はこれらをすべて知らないMMO開発者に会ったこともありますし、この後に述べることの多くがこれなしでは意味をなさないので、簡単に復習しておこうと思います。この文章を短くまとめるために、一般化する必要がありますが、ご容赦ください。

# 2022-02-10

## 1. MMOの開発コストの大半はコンテンツ

コンテンツは以下のようなものです
- クエスト（ミッション、ストーリーライン、スクリプトイベント）
- 3Dエリア（ゾーン、ダンジョン、街、風景などのメッシュ、テクスチャ、ロジック）。
- 戦利品（アイテムのグラフィック、ドロップ率、アイテムのステータス、アイテムの能力）

コンテンツは以下のようなものではありません
- 派手な3Dグラフィックス
- 物理エンジン
- AI
- コアゲームルール

...MMO以外のゲームの大半を占める傾向にある。

10年近く前にRaph Koster（他にもいますが、Raphは明確で簡潔な説明を最初に思いついた一人です）が指摘したように、プレイヤーがコンテンツを消費する速度は、開発者がコンテンツを生成する速度を大幅に上回ります。現代のMMOでは、おそらく50人が純粋にコンテンツ生成に取り組んでいて、おそらく50万人がコンテンツを消費しています。問題は、その50万人がThottbotを使っているので、発見を*共有*し、その数の二乗に比例した割合で消費するのに対し、一般的には開発者の数に正比例した割合で生産されていることです。

この問題を回避する方法はたくさんありますが、そのほとんどは、膨大な量のコンテンツを作成または管理することになります（そのコンテンツを作成する正確な方法について、より狡猾になるだけです）。

# 2022-02-12

## 2. MMOの開発の大半は発売後に行われる

10年前のMMOは6ヶ月から18ヶ月ごとに拡張版やコンテンツのアップデートをリリースしていましたが、現代のMMOは3ヶ月から12ヶ月ごとに新しいコンテンツをリリースしています。ここでいう「新しいコンテンツ」とは、一般に、まったく新しいクエストやまったく新しいアイテムを持つ、まったく新しい3Dエリアを意味し、まったく新しい特殊能力や新しいプロットはもちろんのこと、すなわち完全な「ミニチュアMMO」である。
ほぼすべてのMMOは、その生涯を通じてマイナーアップデートを繰り返してきましたが、唯一書き換えを必要としないのは技術的な部分だけです。しかし......私は以前から、もっと技術のアップデートがあればいいのにと思っていたのですが、現在3大MMOが大規模なアップデートを行なっています。Ultima Online (1997)は最近、10年前のグラフィックをKingdom Rebornとして大規模に改修し、Eve Online (2001)はTrinityという「ピカピカで細かい宇宙船と人型の追加」の更新を完了したばかりです。彼らは「現在のエンジンはGPUが本当に利用できるようになる前に設計された」と言いますが、GeForceが発売されたのはほぼ2年前、TNTやVoodooなどはその何年も前ですから、それはあからさまに嘘です。そう、発売当時は対応する最新のハードウェアが5年前のものだったという苦い経験があります :)) 。いずれもゲームのルック＆フィールに大きな変更を加えたもので、その視覚的インパクトは計り知れないものがあります。Runescapeは、数年前に大規模なアップデート（「Runescape 2」）を行いました。

しかし......いずれも、発売時には最先端ではなかったゲームを、現在の最先端にはまだはるかに及ばない水準にまで引き上げているのです。これは理解することが重要です：グラフィックの品質は、（成功したMMOの展開によると）あなたがおそらく買うことができるほとんどすべての他の成功したゲームとは異なり、これらのゲームのための主要なセールスポイントではありません間違いなくです。

では......発売後に行われる「開発」とは何でしょうか？それはコンテンツであり、エンジンではありません。そして、エンジンであっても（グラフィックの向上、物理演算の改善）、当時のPCやコンソールのスタンドアローンタイトルの基準からすれば、常に最小限の改良にとどまっています。

参考までに、多くのMMOは発売後に、発売までの3～5年の開発期間よりも大規模な開発チームを抱えることになります。すべての追加コンテンツは、多くの人を必要とします（すべての古いコンテンツを動作させ続けることは言うまでもありませんし、新しいコンテンツと競合する場合はそれを更新する等々）。

そのうえで、カスタマーサポートの問題がありますが。従来のゲームは発売したらそれでおしまい。終わった。ちなみに、これはパブリッシャーがMMOから学んだことが大きいのではないかと思う。BethesdaのTES4/Oblivionでは、パッチを有料にしたほどです。

MMOは、月額課金やゲーム内コマースなど、継続的にプレイしてもらうことで収益を得ているため、できるだけ長くプレイしてもらうことが重要です。もしプレイヤーがMMOを6ヶ月ではなく12ヶ月プレイすれば、利益は2倍になります。もしプレイヤーがコンソールタイトルを6ヶ月ではなく12ヶ月プレイすれば、利益はゼロになり、おそらく続編の販売もカニバリゼーション（共食い）してしまうでしょう。

だからMMOにとって生涯総開発費は、発売前の開発費よりも、経済的にずっと重要です。

## 3. コアロジックが現在機能している方法は、将来にわたって機能する方法ではない

遅かれ早かれ、ナーフするんですね。
https://www.urbandictionary.com/define.php?term=nerf

通常、あなた（のプレイヤー）はPvPにおいて、あなたが考えもしなかった不釣り合いに強力なゲーム戦術を発見し、それがアンバランスでゲームを台無しにしているため、それを「修正しなければならない」のです。

つまり、これまでゲームロジックにハードコードしてきたルールを、すべて破ることができなければならないのです。さらに悪いことに、その影響（変更がもたらすすべてのバグと、元々そこにあったが目立った効果がなく、突然目に見える効果を持つようになった目に見えないバグ、新しいセキュリティホールなど）を処理できなければならないことを意味するのです。

ゲームの継続的な開発（発売後）は、初期開発よりも範囲とコストが大きいため、ゲームのライフタイムにおいては、そもそもコアロジックを簡単に作れることよりも、コアロジックを簡単に変更し、その変更の副作用に対応できることの方が重要であると言えます。

## 4. そこそこ成功しているMMOでも、24時間ごとにギガバイトのデータが発生する

MMOでは、プレイヤー一人ひとりの進捗状況を独自に把握し、どのクエストをクリアしたか、どの装備を持っているかなどをゲーム側が記憶しておく必要があります。10万人ものプレイヤーがいれば、そのデータはすぐに蓄積されます。その多くは毎日捨てられますが（例えば、毎日最後に何ヒットポイント持っていたか、という履歴は一般的には必要ありません）、多くは永久に保存されなければなりません（あるクエストを完了したかどうか、あるクエストでは正確にどのように完了したのか）。

例えば、ヴァンガードをプレイしていると自動的に "ブログ "が作成され、レベルアップやモンスターを倒したり、死んだりするたびに更新されるようになっています。

この後者のデータだけでも、24時間ごとに約30Gbの割合で発生している。

最終的にMMOは、膨大なデータベースを保存し、取り出し、更新しなければならない。このサイズ以上のデータベースを管理するソフトウェアを専門に提供する、数十年の歴史を持つ巨大産業がありますが、MMOの開発者はSQLやリレーショナル・プログラミングを好まないのです。リレーショナル・データベースは負荷に対応できますが、リレーショナル・プログラミングはオブジェクト指向プログラミングと根本的に相容れないのです。その結果、データをディスクに書き込まなければならないプログラマーは、それを退屈で、困難で、苛立たしいと感じるようになり、ミスやバグの大幅な増加、機能性の低下（誰も絶対に必要以上に新機能を追加するコードを書いたり追加したりしたくないから）につながってしまうのです。

## MMO開発の優先順位

このように、MMOの開発では、単純な問題の積み重ねによって、長期的な収益性が大きく左右されることがあるのです。

* ゲーム内アイテムの記述に使用するデータを変更するために必要な作業量
* 新しい（古い）ゲーム内アイテムの種類を追加（または削除）するためにどれだけの労力が必要なのか
* コンテンツの再利用性
* 再利用可能なコンテンツを再利用することが、実際にどの程度容易であるか（すなわち、どの程度のコーディングが必要であるか）。
* コンテンツやロジックを修正するために、ゲームコードやゲーム全体のデザイン、ゲームシステムの詳細など、どれだけの専門知識が必要なのか(発売前のゲームをもともと書いていたチームとして、新しいチームメンバーが活躍できるのか?)
* ゲームをプレイすることで発生するデータ（進捗、実績、プレイヤーの履歴など）はどの程度エクスポート可能なのか。
* プレイによって得られたゲームデータの分析性（何を変えるべきかの判断や、ゲームの改良による改善点の確認など）。
* コアコンテンツがどの程度変更可能か
* 個々のルールの変更とその副作用の確認がどれだけ容易か
* 上記のうちいくつの変更が、プログラマーを必要とせずにできるか（デザイナーは自分でコードを変更できるか、アーティストはできるのか？）
* サーバーサイドのサービス提供に、どれだけサードパーティの専門システムを活用できるか。

その中には、あまり効果的でないもの（曖昧すぎる）もあれば、恐ろしくコストのかかるものもあります（ライブゲームに変更を加えるたびに「あらゆるものをテスト」するためには、多くの人が必要です）。また、「プログラマーフレンドリー」でもありません。実際、これらはすべて、非プログラマーの利益のために意図的にプログラマーフレンドリーでないように作られています。

そして、エンティティ・システムが、典型的なMMOの開発プロセスにどのような影響を与え、問題を解決したり、現在のソリューションをよりプログラマフレンドリーにすることができるのかについてです（第5回になります。）

# 2022-02-13

Entity Systems are the future of MMOG development に対する考察。

著者の考えでは、ES はリレーショナルデータベースのようなものなので、
実行時速度を犠牲にしても、リレーションは動的であるべき。つまり、
あるエンティティが保持するコンポーネントは、ゲームの実行中に増えたり減ったりする。
では、あるエンティティとあるシステムの関係は、ゲーム中に変化するか？
おそらく、エンティティがシステムを直接保持している訳ではなく、システムは単に、
そのシステムが対応するコンポーネントを全て操作する。
つまり、「あるエンティティがあるコンポーネントを持つかどうか？」のみ変更する事ができ、
あるコンポーネントを持ったら、それに対応するシステムは必ず適用される。

# 2022-02-14

自作 ECS の動作確認。
EntityManager::remove_system() で動的に System の削除が可能な事を確認。
TODO : System から Component にアクセスする手段を確認する。

# 2022-02-15

# 2022-02-16

System::component_list_ にコンポーネントを追加・削除する処理が必要。
必要があるのは以下のタイミング

* Entity に Component が追加された時
* Entity から Component が削除された時
    * Entity が削除された時を含む

# 2022-02-17

# 2022-02-18

* 自作 ECS の実装。
    * 新しい疑問「System は Component の Tupple の List」を知っているだけで良いのか？
        * つまり、Entity を知る必要は無いのか？
        * => 今の時点での答え : Entity を知る必要は無い。Component の Tupple の List のみで良い
            * 理由 : System が扱うのはあくまで Component であって、そのコンポーネントがどの Entity によって保持されているかは感知しない
            * 本当にそれで良いかは、ゲーム本体の実装を進める中で検証されるはず
    * ひとまず、自身が ComponentType を含むかを調べる System::has_component< ComponentType >() を実装する。

# 2022-02-19

* System::has_component< ComponentType >() はテンプレートパラメータを取るため仮想関数にできない事に気付いた

# 2022-02-20

* ひとまず仮想関数ではない関数テンプレートとして System::has_component< ComponentType >() を実装
    * 良く分からないなりに constexpr, tuple_element, is_same などを使ってやってみた

# 2022-02-21

コンパイルエラー。

* constexpr 関数内でも for ループ変数 n を tuple_element< n, xxx > とテンプレート引数に使うとコンパイルエラー
* Head, Tail で型リストを再帰で回す形式に変えてみたが、実引数が無いため？エラー

# 2022-02-22

* コンパイルエラー。直らず。

# 2022-02-26

* C++ テンプレートについて調査。

以下のようなシンプルなサンプルプログラムを書いてみたが、コンパイルエラー。
実引数が無いとダメか。

```c++
#include <tuple>
#include <iostream>

template< typename Head, typename ... Tail >
constexpr int get_total_size()
{
        return sizeof( Head ) + get_total_size< Tail ... >();
}

template< typename Head >
constexpr int get_total_size< Head >()
{
        return sizeof( Head );
}

int main( int, char** )
{
        std::cout << "1 : " << get_total_size< int >() << std::endl;
        std::cout << "2 : " << get_total_size< int, char >() << std::endl;
        std::cout << "3 : " << get_total_size< int, char, double >() << std::endl;

        return 0;
}
```

```
$ g++ 2.cpp
2.cpp:11:38: error: non-class, non-variable partial specialization ‘get_total_size<Head>’ is not allowed
   11 | constexpr int get_total_size< Head >()
      |                                      ^
2.cpp: In function ‘int main(int, char**)’:
2.cpp:18:47: error: call of overloaded ‘get_total_size<int>()’ is ambiguous
   18 |  std::cout << "1 : " << get_total_size< int >() << std::endl;
      |                                               ^
2.cpp:5:15: note: candidate: ‘constexpr int get_total_size() [with Head = int; Tail = {}]’
    5 | constexpr int get_total_size()
      |               ^~~~~~~~~~~~~~
2.cpp:11:15: note: candidate: ‘constexpr int get_total_size() [with Head = int]’
   11 | constexpr int get_total_size< Head >()
      |               ^~~~~~~~~~~~~~~~~~~~~~
2.cpp: In instantiation of ‘constexpr int get_total_size() [with Head = char; Tail = {}]’:
2.cpp:7:52:   required from ‘constexpr int get_total_size() [with Head = int; Tail = {char}]’
2.cpp:19:53:   required from here
2.cpp:7:52: error: no matching function for call to ‘get_total_size<>()’
    7 |  return sizeof( Head ) + get_total_size< Tail ... >();
      |                          ~~~~~~~~~~~~~~~~~~~~~~~~~~^~
2.cpp:5:15: note: candidate: ‘template<class Head, class ... Tail> constexpr int get_total_size()’
    5 | constexpr int get_total_size()
      |               ^~~~~~~~~~~~~~
2.cpp:5:15: note:   template argument deduction/substitution failed:
2.cpp:7:52: note:   couldn’t deduce template parameter ‘Head’
    7 |  return sizeof( Head ) + get_total_size< Tail ... >();
      |                          ~~~~~~~~~~~~~~~~~~~~~~~~~~^~
2.cpp: In instantiation of ‘constexpr int get_total_size() [with Head = double; Tail = {}]’:
2.cpp:7:52:   recursively required from ‘constexpr int get_total_size() [with Head = char; Tail = {double}]’
2.cpp:7:52:   required from ‘constexpr int get_total_size() [with Head = int; Tail = {char, double}]’
2.cpp:20:61:   required from here
2.cpp:7:52: error: no matching function for call to ‘get_total_size<>()’
2.cpp:5:15: note: candidate: ‘template<class Head, class ... Tail> constexpr int get_total_size()’
    5 | constexpr int get_total_size()
      |               ^~~~~~~~~~~~~~
2.cpp:5:15: note:   template argument deduction/substitution failed:
2.cpp:7:52: note:   couldn’t deduce template parameter ‘Head’
    7 |  return sizeof( Head ) + get_total_size< Tail ... >();
      |                          ~~~~~~~~~~~~~~~~~~~~~~~~~~^~
```

```non-class, non-variable partial specialization ‘get_total_size<Head>’ is not allowed```

と言われたので、class の static 関数にしてみたが、ダメ。

```c++
class Hoge
{
public:
    template< typename Head, typename ... Tail >
    constexpr static int get_total_size()
    {
        return sizeof( Head ) + get_total_size< Tail ... >();
    }

    template< typename Head >
    constexpr static int get_total_size< Head >()
    {
        return sizeof( Head );
    }
};
```

```
$ g++ 3.cpp
3.cpp:13:49: error: non-class, non-variable partial specialization ‘get_total_size<Head>’ is not allowed
   13 |     constexpr static int get_total_size< Head >()
      |                                                 ^
3.cpp: In function ‘int main(int, char**)’:
3.cpp:21:56: error: call of overloaded ‘get_total_size()’ is ambiguous
   21 |     std::cout << "1 : " << Hoge::get_total_size< int >() << std::endl;
      |                                                        ^
3.cpp:7:26: note: candidate: ‘static constexpr int Hoge::get_total_size() [with Head = int; Tail = {}]’
    7 |     constexpr static int get_total_size()
      |                          ^~~~~~~~~~~~~~
3.cpp:13:26: note: candidate: ‘static constexpr int Hoge::get_total_size() [with Head = int]’
   13 |     constexpr static int get_total_size< Head >()
      |                          ^~~~~~~~~~~~~~~~~~~~~~
3.cpp: In instantiation of ‘static constexpr int Hoge::get_total_size() [with Head = int; Tail = {char}]’:
3.cpp:22:62:   required from here
3.cpp:9:59: error: call of overloaded ‘get_total_size()’ is ambiguous
    9 |         return sizeof( Head ) + get_total_size< Tail ... >();
      |                                 ~~~~~~~~~~~~~~~~~~~~~~~~~~^~
3.cpp:7:26: note: candidate: ‘static constexpr int Hoge::get_total_size() [with Head = char; Tail = {}]’
    7 |     constexpr static int get_total_size()
      |                          ^~~~~~~~~~~~~~
3.cpp:13:26: note: candidate: ‘static constexpr int Hoge::get_total_size() [with Head = char]’
   13 |     constexpr static int get_total_size< Head >()
      |                          ^~~~~~~~~~~~~~~~~~~~~~
3.cpp: In instantiation of ‘static constexpr int Hoge::get_total_size() [with Head = char; Tail = {double}]’:
3.cpp:9:59:   required from ‘static constexpr int Hoge::get_total_size() [with Head = int; Tail = {char, double}]’
3.cpp:23:70:   required from here
3.cpp:9:59: error: call of overloaded ‘get_total_size()’ is ambiguous
    9 |         return sizeof( Head ) + get_total_size< Tail ... >();
      |                                 ~~~~~~~~~~~~~~~~~~~~~~~~~~^~
3.cpp:7:26: note: candidate: ‘static constexpr int Hoge::get_total_size() [with Head = double; Tail = {}]’
    7 |     constexpr static int get_total_size()
      |                          ^~~~~~~~~~~~~~
3.cpp:13:26: note: candidate: ‘static constexpr int Hoge::get_total_size() [with Head = double]’
   13 |     constexpr static int get_total_size< Head >()
      |                          ^~~~~~~~~~~~~~~~~~~~~~
```

#

まさに欲しかった情報が見つかった。
「C++ template の（部分）特殊化ができるとき、できないとき」

https://mimosa-pudica.net/cpp-specialization.html

こちらを参考に、関数テンプレートからクラステンプレートに変更したらコンパイルが通った。
最適化によって関数呼び出しも無くなり、単に 22 ( 0x16 ) を return する処理になった。

```c++
#include <iostream>

template< typename Head, typename ... Tail >
struct Hoge
{
    static int get_total_size()
    {
        return sizeof( Head ) + Hoge< Tail ... >::get_total_size();
    }
};

template< typename Head >
struct Hoge< Head >
{
    static int get_total_size()
    {
        return sizeof( Head );
    }
};

int main( int, char** )
{
    return
        Hoge< int >::get_total_size() +
        Hoge< int, char >::get_total_size() + 
        Hoge< int, char, double >::get_total_size();
}
```

```
g++ 6.cpp -O4 -g -c
objdump 6.o -S
```

```
6.o:     file format elf64-x86-64


Disassembly of section .text.startup:

0000000000000000 <main>:
        return sizeof( Head );
    }
};

int main( int, char** )
{
   0:   f3 0f 1e fa             endbr64

    return
        Hoge< int >::get_total_size() +
        Hoge< int, char >::get_total_size() +
        Hoge< int, char, double >::get_total_size();
}
   4:   b8 16 00 00 00          mov    $0x16,%eax
   9:   c3                      retq
   a:   66 0f 1f 44 00 00       nopw   0x0(%rax,%rax,1)

0000000000000010 <_GLOBAL__sub_I_main>:
  10:   f3 0f 1e fa             endbr64
  14:   48 83 ec 08             sub    $0x8,%rsp
  extern wostream wclog;        /// Linked to standard error (buffered)
#endif
  //@}

  // For construction of filebuffers for cout, cin, cerr, clog et. al.
  static ios_base::Init __ioinit;
  18:   48 8d 3d 00 00 00 00    lea    0x0(%rip),%rdi        # 1f <_GLOBAL__sub_I_main+0xf>
  1f:   e8 00 00 00 00          callq  24 <_GLOBAL__sub_I_main+0x14>
  24:   48 8b 3d 00 00 00 00    mov    0x0(%rip),%rdi        # 2b <_GLOBAL__sub_I_main+0x1b>
  2b:   48 83 c4 08             add    $0x8,%rsp
  2f:   48 8d 15 00 00 00 00    lea    0x0(%rip),%rdx        # 36 <_GLOBAL__sub_I_main+0x26>
  36:   48 8d 35 00 00 00 00    lea    0x0(%rip),%rsi        # 3d <_GLOBAL__sub_I_main+0x2d>
  3d:   e9 00 00 00 00          jmpq   42 <_GLOBAL__sub_I_main+0x32>
```

VS Code でも以下を参照して逆アセンブラしたコードが表示できるようにした。

* https://code.visualstudio.com/docs/cpp/config-msvc
* https://devblogs.microsoft.com/cppblog/visual-studio-code-c-july-2021-update-disassembly-view-macro-expansion-and-windows-arm64-debugging/

# 2022-02-28

* System::has_component_type() のコンパイルに成功。

# 2022-03-01

* Google Test を導入
* blue-sky-2 本体のコードをどうリンクするかが課題。
    * 現状、 *.cpp に書いてあるコードはテストできない。
    * 案
        * blue-sky-2-google-test にソースコードをぶち込む
            * 良くない
        * blue-sky-2 を .lib と exe に分けて、blue-sky-2-google-test から blue-sky-2.lib をリンクする
        * blue-sky-2.exe にリンクする事はできないのか？

# 2022-03-02

* BaseSystem::on_remove_component() を実装。
    * Entity から Component が取り除かれた時に、正しく System からも Component が取り除かれるようになった。

# 2022-03-03

* System 追加時、 System に必要な Component への参照が正しく追加されるように修正。

# 2022-03-04

* システムに優先度を設定できるようにした。

# 2022-03-05

* Test のためのプロジェクト設定見直し
    * blue-sky-2 プロジェクトの「構成の種類」を「アプリケーション(.exe)」から「スタティックライブラリ(.lib)」にすると、
        * blue-sky-2-google-test プロジェクトの追加の依存ファイルに何も設定しなくても、リンクが成功した。
    * blue-sky-2 プロジェクトの「構成の種類」が「アプリケーション(.exe)(.exe)」だと
        * blue-sky-2-google-test から blue-sky-2 の .cpp に書かれている処理が見つけられず、リンクエラーとなる

# 2022-03-06

* Test のためのプロジェクト設定変更
    * blue-sky-2 プロジェクトを .lib に変更
    * 新しく .exe を生成するための blue-sky-2-exe プロジェクトを作成
    * blue-sky-2 では警告だったものが blue-sky-2-exe ではエラーとなる場合があるため、要調査

# 2022-03-07

* Test のためのプロジェクト設定変更
    * blue-sky-2-exe プロジェクトの設定が C++14 だったので C++17 に修正
    * common::auto_ptr のエラーがまだ出る
    * default を仮引数名として使っていた common Config.h がエラーになったため修正

# 2022-03-08

* Test のためのプロジェクト設定変更
    * blue-sky-2-exe プロジェクトの設定で「準拠モード」を「なし」に変えてみたが、common::auto_ptr のエラーは消えず
    * common に auto_ptr と safe_ptr を見つけ、両方 blue-sky から使われており、違いが分からなかったため調査
        * auto_ptr と safe_ptr のコメントに違いを明記した

# 2022-03-09

* Test のためのプロジェクト設定変更
    * VS 2022 でビルドするようになった事が関係しているのかと思って、Debug/ Release/ ディレクトリを削除し、 blue-sky-2 をリビルドしてみた
        * 結果はエラーなし
            * つまり、blue-sky-2 プロジェクトの設定 + VS 2022 でのビルドは問題無し
    * blue-sky-2 プロジェクトをコピーし blue-sky-2-exe-2 プロジェクトとして「構成の種類」を「アプリケーション(.exe)」に変えるだけしてビルドしてみた
        * 結果は common::auto_ptr の deprecated エラーが出てしまう
            * 設定は同じはずなのに、なぜ？

# 2022-03-10

* Test のためのプロジェクト設定変更
    * 再度、blue-sky-2 プロジェクトをコピーして blue-sky-2-exe-2 プロジェクトを作ってみた
        * 昨日は deprecated がエラーになった以外に、リンクのエラーが出ていた
        * プロジェクトに lib.cpp を追加
        * 結果はエラーなし
        * 昨日の deprecated エラーは消えた。なぜ？
    * このまま進むか？
    * やはり新規にプロジェクトを作り、そこでエラーが出ないようにソースを修正した方が安全では？
    * プロジェクトファイル、ソリューションファイルの配置も整理したい。
        * 整理してみた
            * 問題が出た
                * build/ にソリューションファイル・プロジェクトファイルを移動して ../source/ のソースコードを参照すると
                    * build/Win32/Debug/ を中間ディレクトリ設定したはずが build/Win32/source に中間ファイルができてしまう ( ソースへの相対パスが ../source/ で始まるため )
                * source/ にソリューションファイル・プロジェクトファイルを移動すると謎のエラー
                    * Main.obj : error LNK2019: 未解決の外部シンボル "private: __thiscall App::App(void)" (??0App@@AAE@XZ) が関数 "public: static class App * __cdecl common::Singleton<class App>::get_instance(void)" (?get_instance@?$Singleton@VApp@@@common@@SAPAVApp@@XZ) で参照されました
                    * 基本的に、複数のプロジェクトを同じディレクトリに設置すると、問題が起こりやすいのではないか？
                        * 以下のように、プロジェクトごとにディレクトリを分けて、各ディレクトリにプロジェクトファイルを設置するべきか？
                            * source/blue-sky/
                            * source/blue-sky-exe/
                            * source/blue-sky-test/

# 2022-03-11

* Test のためのプロジェクト設定変更
    * 対応完了！！！
    * blue-sky.sln blue-sky.vcproj, blue-sky-exe.vcproj, blue-sky-test.vcproj を source/ に配置した
    * ビルド時に仕様される出力用ディレクトリ、中間ファイル用ディレクトリは全て .build/ に統一した
    * 今後、 source/ 直下に配置されている .h, .cpp を サブディレクトリに整理していく ( おそらく大半は blue_sky/ 以下に移動 )
    * なぜかビルドした実行ファイルのウィンドウサイズが 1.5 倍ぐらいに大きくなっている気がする ( 開発環境の画面設定が 150% になっているが、それが反映されるようになったのか？ )

# 2022-03-12

* プロジェクト整理
    * source/ 直下のファイルを移動して整理。

# 2022-03-14

* ECS を使ったパーティクルシステムの実装
    * まずは ParticleSystemTestScene において Camera の挙動を Entity + Component + System を使ったものに置き換える。
    * TransformComponent の実装が必要。

# 2022-03-15

* ParticleSystemTestScene を ECS を使った実装にするために、まずは Camera を ECS に置き換え中。
    * btTransform から forward, right, up を取り出す方法について調べる必要がある

# 2022-03-16

* btVector, btTransform を理解・テストするために BulletTest.cpp を test プロジェクトに作成。

# 2022-03-21

* 座標系について調べた。Bullet は右手座標系。 DirectX は左手座標系なので、変換について考える必要がある。
    * https://github.com/bulletphysics/bullet3/blob/master/docs/Bullet_User_Manual.pdf

# 2022-03-22

* ParticleSystemTestScene で set_identity() を呼ぶのを忘れていた事に起因するバグを修正。

# 2022-03-23

* ParticleSystemTestScene マウスによるカメラの角度変更動作を調整。動いてはいるが、 roll pich yow について、正しく使えているか要確認。

# 2022-03-24

* Quaternion() の引数は、 yaw_z, pitch_y, roll_x の順番となっている。現状、 mouse dx を yaw_z に渡しているが、おかしい気がする。

# 2022-03-26

* 「Bullet は右手座標系を使用している」
    * https://github.com/bulletphysics/bullet3/blob/master/docs/Bullet_User_Manual.pdf
* その図によると、X が右, Y が上, Z は手前。
    * Z が上なのでは？という心配しなくてよさそう？

# 2022-04-06

* 左手座標系である DirectX も、右手座標系である Bullet も、「原点に向かって右回りがプラスの回転」という点は同じらしい
* DirextXMath の回転をテストするテストコードを作って原点に向かって右回りがプラスの回転である事を確認した。

# 2022-04-07

* 昨日書いた回転の向きの解釈は間違い。正しくは、以下の通り
    * DirectX は「原点に向かって右回りがプラスの回転」
    * Bullet は「原点に向かって左回りがプラスの回転」
* DirectXMath と Bullet で Z 軸まわりの回転を比較

# 2022-04-08

* DirectXMath, Bullet のテストを修正した
* 溜まっていた変更をコミット
* 今後は毎日細かくコミットした方が良い

# 2022-04-09

* DirectXTK の SimpleMath の Quaternion を確認
    * データの保持には XMFLOAT4 を使っている
        * というか XMFLOAT4 を継承している
        * https://github.com/microsoft/DirectXTK/blob/main/Inc/SimpleMath.h#L683
* blue_sky にも direct_x_math::Quaternion を作るべきか？
    * SimpleMath を使うのもアリかも

# 2022-04-10

* GameObject::transform_ に対して set_rotation() しているのはどこか？既存のソースを調べてみた。結果は以下の通り
    * ActiveObject::restart()
    * ActiveObject::set_start_rotation()
    * ActiveObject::set_direction_degree()
    * どれも毎フレームなど頻繁に呼ばれているようではない。
    * Transform の保持は DirectXMath の Quaternion で保持して、Bullet が物理演算する時だけ Bullet に回転を設定するようにできるか？要検討


# 2022-04-11

* 昨日の考えについて今の時点での結論
    * 基本的に DirectXMath が回転を保持して「Bullet が物理演算する時だけ Bullet に回転を設定する」よりも・・・
    * 基本的に「Bullet が回転を保持して、描画など、必要な時だけ取り出す」とした方がよさそう
    * 理由は、
        * 直接回転角度を設定するような事は、毎フレーム起こらない。
        * Bullet の物理演算は基本的に毎フレーム行われる
        * 描画も毎フレーム行われる
        * 毎フレーム Bullet <-> DirectX の変換が必要であれば、どちらでも同じ？

# 2022-04-12

* Unity がどうやって回転を保持しているのか？
    * たとえば Rotation X に 0.123456789 を入力すると 0.1234568 と変更される
    * Position X も同様で 0.123456789 を入力すると 0.1234568 と変更される

# 2022-04-13

* 左手座標系 Y Up で core::bullet を整理したいが、正しく動いているかの判断のため、画面を描画しながらやりたい
    * まずは Cube などを表示するテスト用の Scene を新たに作るか。

# 2022-04-15

* TransformTestScene を作った。

# 2022-04-16

* TransformTestScene に ImGui で Position と Rotation を設定できるようにした。

# 2022-04-18

* RenderSystem, RenderComponent を追加し、簡単なモデルを ECS を使って表示するように。

# 2022-04-19

* Unity の PureECS について調べた
    * https://docs.google.com/presentation/d/11ej0BfUxs7eyFeGGplJJR5v-UgVAZt-L-1EBCLLw5FA/htmlpresent
    * https://qiita.com/mao_/items/c4a217b33a0a94d5d52f
    * デフォルトでは Position, Rotation, Scale が分かれているらしい

# 2022-04-21

* TransformTestScene で ImGui で設定した Position と Rotation がテストで表示されている「壁」オブジェクトに反映されるようにした。

# 2022-04-22

* ImGui 検証
    * DragFloat3 の引数 format で Prefix を設定できるが、
        * 例えば "pos : %.3f" とすると
            * "pos : 0.000  pos : 0.000  pos : 0.000" のような表示にはできる
            * "x : 0.000  y : 0.000  x : 0.000" のような表示はできない
            * 似たような疑問が GitHub Issues でも出ているが、対応していないよう
                * https://github.com/ocornut/imgui/issues/1831

# 2022-04-23

* 回転順序について調査
    * 現状 blue-sky は ZYX の順のよう
        * setEulerZYX() を使っているから多分そう
        * 動作させてみると、 Unity とは異なる結果になった
        * エディタでは、Z -> Y -> X の順で回転させると、回転後の軸を使って次の回転を行える

    * Unity は ZXY の順らしい
        * https://forum.unity.com/threads/rotation-order.13469/
        * https://docs.unity3d.com/ScriptReference/Transform-eulerAngles.html
            * In Unity these rotations are performed around the Z axis, the X axis, and the Y axis, in that order
        * エディタでは、Y -> X -> Z の順で回転させると、回転後の軸を使って次の回転を行える
    * Unreal Engine は ZYX の順っぽい
        * そもそも座標系が違うので、比較する意味はないかも？
            * Unity, DirectX と同じく左手座標系だが、
            * X+ が奥, Y+ が右, Z+ が上？
        * エディタでは、Z -> Y -> X の順で回転させると、回転後の軸を使って次の回転を行える
* 回転方向について調査
    * Unity は回転軸に沿って原点を見た時に右回り ( = 時計回り ) が + の回転
    * Unreal Engine は X, Y と Z で異なる？？？
        * X, Y : 回転軸に沿って原点を見た時に、左回り ( = 反時計回り ) が + の回転
        * Z : 回転軸に沿って原点を見た時に、右回り ( = 時計回り ) が + の回転
    * blue-sky は回転軸に沿って原点を見た時に右回り ( = 時計回り ) が + の回転

# 2022-04-24

* 今後の方向性について検討
    * 回転順序、回転方向については、一旦このままとする。
        * 問題が起こったら再度検討する。

# 2022-04-25

* Scene::update() と Scene::render() の統合について検討
* 全てを ECS に移行すれば UpdateSystem, RenderSystem() に分けられるはず？
* 試しに、TransformTestScene::render() の中の処理を、新しく作る System に移動して、 TransformTestScene::render() を空にしてみてはどうか？
    * TransformTestScene::render() だけでなく、TransformTestScene::update() の中の処理も移動する必要がある。
        * なぜなら、get_entity_manager()->update() はループにつき 1 回しか呼ばれないため、全てを ECS で処理する必要がある。
        * TransformTestScene::update() の中で更新処理のために 1 回、 TransformTestScene::render() の中で描画のために 1 回呼んでしまっていて、非常に汚い。 ( 仮ではあるが )
    
# 2022-04-30

* TransformTestScene::update() にあったキーボードとマウスの入力によってカメラを移動・回転させる処理を TransformControlComponent / TransformControlSystem に移動。

# 2022-05-01

* 西田さんと blue-sky 制作日誌 #1 配信。西田さんと以下について話した
    * Unreal Engine 4 の回転方向
    * 複素数, 三角関数, ベクトル, 行列
        * 虚数・複素数について
            * https://atarimae.biz/archives/500
    * 西田さんに blue-sky の ECS について説明。以下の意見をもらった
        * メモリの断片化に注意 ( 特にコンソールを意識する場合、仮想メモリがない？ため、new に失敗するかもしれない ) とのアドバイスあり

# 2022-05-02

* TransformTestScene を ECS に移行
* 以下の処理が必要
    1. 全 Entity の位置情報などの更新 ( update )
    2. カメラの情報を Shader に送る
        * そのための方法
            1. **GraphicsManager::set_main_camera() みたいな関数を作る**
                * System が増えなくて済む。 RenderSystem だけでレンダリングできる
                * Entity から Component を取得するのは時間がかかるので、GraphicsManager が Camera の Entity ではなくて、 TransformComponent と CameraComponent を持つ必要がある
            2. ~~CameraRenderSetupSystem みたいな System を作る_~~
                * 動的にメインカメラが変わっても、自動的に追従する
                * System が増える
    3. 全 Entity の描画
    * これらは全て System が行う
* 本来ならば、GraphicsManager がメインカメラの情報を持つべきなのでは？
    * GraphicsManager::set_main_camera() みたいな関数を作るか
* CameraComponent を作った
* GraphicsManager::set_main_camera() を作った。
* TransformTestScene::render() の中の処理を、 RenderSystem に移動できた！

# 2022-05-03

* TransformTestScene でのテキスト描画の不具合を修正する
    * Sprite 用の RASTERIZERSTATE の設定が抜けていたせいで TransformTestScene で Text が正しく描画できていなかった問題を修正した。
* EntityManager::clear() を実装した

# 2022-05-04

* ModelComponent を作った
* TransformTestScene で GUI で任意のモデルを読み込めるようにした
* 任意のモデルを ECS で表示できるようになった。

# 2022-05-05

* カメラの情報を GUI で変更できるようにした

# 2022-05-06

* App.cpp, GameMain.cpp のコンパイラ警告に対応。

# 2022-05-07

* App, GameMain を整理しマウスカーソル表示・非表示切り替えの不具合を修正した。

# 2022-05-08

* Shader に get_name() を実装。

# 2022-05-09

* Shader を GUI で変更できるようにした。

# 2022-05-10

* コマンドモードで ESC キーによる入力を無効にした

# 2022-05-12

* Clang でコンパイルした際のエラー・警告に一部対応。

# 2022-06-06

* 手描き風の表現について検討
    1. バッファに Material ID のようなものを描画
    2. 描画されたバッファを歪ませる
    3. 歪んだバッファの Material ID 参照して ID 毎に異なる素材のテクスチャを描画
    * Texture2DArray が使えそう
        * https://docs.microsoft.com/ja-jp/windows/win32/direct3dhlsl/texture2darray-sample
        * 最大何枚のテクスチャを 1 回の Draw で使えるのだろう？

# 2022-06-07

* 昨日思いついた手描き風の表現のためには、複数回の描画が必要なため、複数回 Post Effect を行える仕組みを検討

```c++
void test()
{
    // フォーマットを指定してテクスチャを作成
    // 第 2, 第 3 引数を省略すると画面幅 * 画面高さのテクスチャを作成してくれる
    auto a = new RenderTargetTexture( PixelFormat::R8_UINT );
    auto b = new RenderTargetTexture( PixelFormat::R8G8B8A8_UNORM );

    // a に世界を描画
    get_graphics_manager()->set_render_target( a );
    
    get_graphics_manager()->render_xxx();
    get_graphics_manager()->render_xxx();

    // a から b に shader_a_to_b を使ってポストエフェクトを描画
    get_graphics_manager()->set_post_effect_shader( shader_a_to_b );
    get_graphics_manager()->render_post_effect( a, b );

    // b から back buffer に shader_b_to_back_buffer を使ってポストエフェクトを描画
    get_graphics_manager()->set_post_effect_shader( shader_b_to_back_buffer );
    get_graphics_manager()->render_post_effect( b );
}
```

# 2022-06-09

* Unity での 複数の Post Effect の設定方法を調査
    * Unity のポストプロセッシングには 3 種類ある。レンダーパイプラインの種類に対応している。
        * ビルトインレンダーパイプライン
        * ユニバーサルレンダーパイプライン (URP)
        * HD レンダーパイプライン (HDRP)
        * https://docs.unity3d.com/ja/2020.3/Manual/PostProcessingOverview.html
    * 今回は、ビルトインレンダーパイプライン + Post Processing でのポストエフェクトの設定の方法を確認した
        * MainCamera に Post-processing コンポーネントを追加し、それに対して各エフェクトを追加できる
            * 簡単なシーンを作って Stats で描画負荷を確認
                * Bloom を有効にすると、 Batches と SetPass calls が 13 増えた
                * Color Grading を有効にしても、Batches と SetPass calls は増えなかった
                * Ambient Occlusion を有効にすると、 Batches と SetPass calls が 2 増えた
        * https://docs.unity3d.com/Packages/com.unity.postprocessing@3.2/manual/index.html
        * この形であれば、 6/7 に検討した仕組みで blue-sky でも実現できそう
    * 現在のプロジェクトのレンダーパイプラインの確認・変更方法
        * https://docs.unity3d.com/ja/2020.3/Manual/srp-setting-render-pipeline-asset.html

# 2022-06-10

* RenderSystem で仮で 手描き風ポストエフェクトと色収差ポストエフェクトの 2 つを適用
* 1 pixel 8 bit のテクスチャに VertexID を書き込み、その後ポストエフェクトで色に変換するテストを実装

# 