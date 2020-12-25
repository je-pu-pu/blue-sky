# 外部ライブラリの準備

## ディレクトリ構成

    include/                            .h, .hpp ファイル
    x86/                                32 bit
        common/                         debug, release の区別の無い .lib ファイル
        debug/                          デバッグ用 .lib ファイル
        release/                        リリース用 .lib ファイル

## 共通コード生成オプション

* デバッグ : マルチスレッド デバッグ ( /MTd )
* リリース : マルチスレッド ( /MT )

## boost

1. ```bootstrap.bat``` を実行する
2. ```./b2.exe runtime-link=static``` を実行する
3. ```stage/lib/``` 以下の必要なファイルを ```blue-sky/source/lib/``` 以下の適切なディレクトリにコピーする。