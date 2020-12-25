blue-sky
========

blue-sky は 1 人称視点のアクションゲームです。  
空を飛ぶ感覚を味わえるゲーム目指して開発中です。

![blue-sky](http://game.je-pu-pu.jp/blue-sky/images/blue-sky-title-560x420.png)

http://game.je-pu-pu.jp/blue-sky/

![blue-sky 2](http://game.je-pu-pu.jp/blue-sky-2/images/title-800x500.png)

http://game.je-pu-pu.jp/blue-sky-2/


プラットフォーム
========
Windows 10 ( 32bit )

使っているライブラリとバージョン
========

- Direct3D11
- DirectWrite
- DirectSound
- DirectInput
- [imgui 1.75](https://github.com/ocornut/imgui/releases/tag/v1.75)
- [lua 5.3.4](https://www.lua.org/ftp/)
- [sol2 3.2.3](https://github.com/ThePhD/sol2/tree/v3.2.3)
- [FBX SDK 2020.2 VS2019](https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2020-2)
- [Oculus SDK for Windows 1.43.0](https://developer.oculus.com/downloads/package/oculus-sdk-for-windows/1.43.0/)
- [boost 1.6.7](https://www.boost.org/)
- [bullet3 2.87](https://github.com/bulletphysics/bullet3/releases/tag/2.87)
- [libogg 1.3.4](https://xiph.org/downloads/)
- [libvorbis 1.3.7](https://xiph.org/downloads/)
- [Effects for Direct3D 11 (FX11) jun2020](https://github.com/microsoft/FX11/tree/jun2020)

ディレクトリ構成
========
    
    blue-sky/
        doc/                            ドキュメント ( 文書 )
            html/                           Doxygen によって生成されたドキュメント
        log/                            テストプレイヤーから収集したログ
        manual/                         説明書 HTML
        media/                          素材 ( 3D モデル・テクスチャ・サウンド )
        package/                        リリースするパッケージ
        source/                         ソースコード
            blender_script/             　 　　Blender 用 python スクリプト
            blue_sky/                   　 　　blue-sky 固有のソースコード
                graphics/               　 　　    グラフィックス関連
                    Direct3D11/         　 　　        Direct3D11
                    shader/             　 　　        シェーダークラス
            core/                       　 　　ゲームエンジンのソースコード
                graphics/               　 　　    グラフィックス関連
                    Direct3D11/         　 　　        Direct3D11
                    DirectWrite/        　 　　        DirectWrite
                input/                  　 　　    コントローラー関連
                    DirectInput/        　 　　        DirectInput
                math/                   　 　　    数学
                    DirectXMath/        　 　　        DirectXMath
                physics/                　 　　    物理
                    Bullet/             　 　　        Bullet physics
                sound/                  　 　　    サウンド
                    DirectSound/        　 　　        DirectSound
            extern/                     　 　　外部プロジェクト
                common/                 　 　　    共通 C++ ライブラリ
                game/                   　 　　    抽象ゲームライブラリ
                type/                   　 　　    共通型定義
                win/                    　 　　    Windows ライブラリ
            GameObject/                 　 　　ゲーム内のオブジェクト
            log/                        　 　　ログファイルを配置するディレクトリ
            media/                      　 　　素材を配置するディレクトリ
            save/                       　 　　セーブファイルを配置するディレクトリ
            Scene/                      　 　　ゲーム内のシーン