# x64 依存ライブラリのセットアップ

blue-sky の **x64(64bit) ビルド**に必要な third-party ライブラリ（`source/lib/x64/`）と
boost ヘッダ（`source/lib/include/boost/`）は **git 管理外**（巨大・再生成可能）。
本ディレクトリはそれらを再構築する手順とスクリプト。

> 背景: x64 化は NPR のリアルタイム実行（ONNX Runtime + DirectML が x64 専用）の前提。
> 経緯と判断は `tasks/todo.md` の「blue-sky 64bit 化（x64ポート）」節を参照。

## 前提ツール

- **Visual Studio 18**（v145 ツールセット）。`cl`/`lib`/`MSBuild` は署名済みなので Device Guard でブロックされない。
- **CMake**: VS 同梱を使用（`…\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe`）。
- **7-Zip**（`C:\Program Files\7-Zip\7z.exe`）: アーカイブ展開。
- `git` / `curl`。

## ① 自動: OSS ライブラリ + boost のビルド

```bash
bash tools/x64-deps/build_oss_libs.sh
```

これで以下を取得・x64 静的(/MTd・/MT)ビルドし、`source/lib/x64/{debug,release}` へ配置:

| ライブラリ | バージョン | 配置名 | 備考 |
|---|---|---|---|
| libogg | 1.3.5 | `libogg.lib` | |
| libvorbis | 1.3.7 | `libvorbis_static.lib` / `libvorbisfile_static.lib` | ogg を指定 |
| Bullet | 3.25 | `BulletCollision/Dynamics/LinearMath_vs2010(_debug).lib` | `USE_MSVC_RUNTIME_LIBRARY_DLL=OFF` |
| PortAudio | 19.7.0 | `portaudio_static.lib` | `PA_USE_WDMKS=OFF`（重複シンボル回避） |
| Effects11(FX11) | master | `effects11d.lib` / `effects11.lib` | |
| Lua | 5.4.3 | `lua.lib` | ヘッダが 5.4.3。**必ず静的CRT**（既存 /MD 版は CRT 不一致でリンク不可） |
| boost | 1.90.0 | `libboost_{filesystem,chrono,timer,regex,atomic}-vc143-mt-(s)gd-x64-1_90.lib` | ヘッダも `source/lib/include/boost` へ配置 |

### ハマりどころ（重要）
- **Device Guard**: 自前ビルドの未署名 exe（boost の `b2.exe` 等）は実行ブロックされる。
  → boost は b2 を使わず **自作 CMakeLists**（`cl`/`link` 経由）でビルドする。
- **新しい cmake**: 古い `cmake_minimum_required` を拒否 → `-DCMAKE_POLICY_VERSION_MINIMUM=3.5`。
- **静的CRT**: `CMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>` だけでは
  古いプロジェクトで無視される（CMP0091 OLD）→ `-DCMAKE_POLICY_DEFAULT_CMP0091=NEW` を併用。
- boost の自動リンク名のツールセットタグは VS18 で **vc143**。
- `NoDefaultCurrentDirectoryInExePath` が有効だと bat の相対呼び出しが失敗する（参考）。

## ② 手動: FBX SDK（Autodesk・要インストール）

1. **Autodesk FBX SDK 2020.3.9 (VS2022)** を既定パスへインストール
   （`C:\Program Files\Autodesk\FBX\FBX SDK\2020.3.9`）。
2. x64 の `-mt`（静的CRT）lib と DLL を配置:
   ```bash
   SDK="/c/Program Files/Autodesk/FBX/FBX SDK/2020.3.9/lib/x64"
   DST="source/lib/x64"
   for cfg in debug release; do
     for f in libfbxsdk-mt.lib libxml2-mt.lib zlib-mt.lib libfbxsdk.dll; do
       cp "$SDK/$cfg/$f" "$DST/$cfg/$f"
     done
   done
   ```
3. 実行時用に `libfbxsdk.dll` を exe 出力先（`source/.build/x64/Debug` 等）へコピー。

※ 2020.3.9 は x86 lib を同梱しない。ヘッダは現状 2020.2 のままだが x64 リンクは通る。

## ビルド

```powershell
& $msbuild source\blue-sky.sln -p:Configuration=Debug -p:Platform=x64 -v:minimal
```
