blue-sky
========

blue-sky は 1 人称視点のアクションゲームです。  
空を飛ぶ感覚を味わえるゲーム目指して開発中です。

![blue-sky](http://game.je-pu-pu.jp/blue-sky/images/blue-sky-title-560x420.png)

http://game.je-pu-pu.jp/blue-sky/

![blue-sky 2](http://game.je-pu-pu.jp/blue-sky-2/images/title-800x500.png)

http://game.je-pu-pu.jp/blue-sky-2/


使っているライブラリなど
========

- Direct3D11
- DirectWrite
- DirectSound
- DirectInput
- OggVorbis
- Bullet Physics
- Oculus Rift

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