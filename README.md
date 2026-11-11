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
- [imgui v1.91.0](https://github.com/ocornut/imgui/releases/tag/v1.91.0)
- [lua 5.3.4](https://www.lua.org/ftp/)
- [sol2 3.2.3](https://github.com/ThePhD/sol2/tree/v3.2.3)
- [FBX SDK 2020.2 VS2019](https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2020-2)
- [Oculus SDK for Windows 1.43.0](https://developer.oculus.com/downloads/package/oculus-sdk-for-windows/1.43.0/)
- [boost 1.6.7](https://www.boost.org/)
- [bullet3 3.25](https://github.com/bulletphysics/bullet3/releases/tag/3.25)
- [libogg 1.3.4](https://xiph.org/downloads/)
- [libvorbis 1.3.7](https://xiph.org/downloads/)
- [Effects for Direct3D 11 (FX11) jun2020](https://github.com/microsoft/FX11/tree/jun2020)
- [portable-file-dialogs 0.0.1](https://github.com/samhocevar/portable-file-dialogs)
- [portaudio 19.7.0](https://github.com/PortAudio/portaudio/releases/tag/v19.7.0)
- [libremidi 5.1.0](https://github.com/celtera/libremidi/releases/tag/v5.1.0)
- [TinySoundFont](https://github.com/schellingb/TinySoundFont)

MIDI の音源（SoundFont）の入手
========

MIDI を鳴らす音源 `source/media/music/FluidR3_GM.sf2`（Fluid R3 GM、Frank Wen、MIT ライセンス、約 142 MB）は大きいので git に入れていません。リポジトリの根で次を実行して入手してください（Python の標準ライブラリだけで動きます）。

    python -I tools/soundfont/fetch_soundfont.py

Debian の `fluid-soundfont-gm_3.1-6_all.deb` をダウンロードし、.deb と取り出した SF2 の SHA-256 を確かめてから置きます。

- .deb の SHA-256 : `9965fbcc6acee17d6f72b685d28c7f968ec64eba14645445b476d5c6cc2eee4c`
- `FluidR3_GM.sf2` の SHA-256 : `74594e8f4250680adf590507a306655a299935343583256f3b722c48a1bc1cb0`（148,398,306 バイト）

入手元・ライセンス・選んだ理由は `doc/ライセンス.md` の 4.7 にあります。

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
            blender_script/                 Blender 用 python スクリプト
            blue_sky/                       blue-sky 固有のソースコード
                graphics/                       グラフィックス関連
                    Direct3D11/                     Direct3D11
                    shader/                         シェーダークラス
            core/                           ゲームエンジンのソースコード
                graphics/                       グラフィックス関連
                    Direct3D11/                     Direct3D11
                    DirectWrite/                    DirectWrite
                input/                          コントローラー関連
                    DirectInput/                    DirectInput
                math/                           数学
                    DirectXMath/                    DirectXMath
                physics/                        物理
                    Bullet/                         Bullet physics
                sound/                          サウンド
                    DirectSound/                    DirectSound
            extern/                         外部プロジェクト
                common/                         共通 C++ ライブラリ
                game/                           抽象ゲームライブラリ
                type/                           共通型定義
                win/                            Windows ライブラリ
            GameObject/                     ゲーム内のオブジェクト
            log/                            ログファイルを配置するディレクトリ
            media/                          素材を配置するディレクトリ
            save/                           セーブファイルを配置するディレクトリ
            Scene/                          ゲーム内のシーン
