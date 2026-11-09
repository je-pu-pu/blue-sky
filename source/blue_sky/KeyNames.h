#pragma once

#include <blue_sky/type.h>

#include <vector>

namespace blue_sky::key_names
{

/**
 * キー名 ⇔ Windows 仮想キーコード（VK_*）の対応表
 *
 * 設定ファイル（`input.key.left A` 等）と、キー割り当て変更 UI で使う。
 *
 * - 名前は大文字・小文字を区別しない（`space` も `SPACE` も可）
 * - 英字・数字 1 文字はそのまま（`A` → 'A'、`1` → '1'）。従来の設定ファイルと互換
 * - 表示用の日本語名は UI 側で文字列テーブル `key.<名前>` を引く（未登録ならこの名前をそのまま表示）
 */

/// 名前から仮想キーコードを得る。不明なら 0
int code_from_name( const string_t& name );

/// 仮想キーコードから名前を得る。不明なら空文字列
string_t name_from_code( int code );

/// キー割り当てに使えるキーの一覧（ESC = キャンセル、F1 / F2 / F5 = グローバルなホットキー は含まない）
const std::vector< int >& get_bindable_codes();

/// 割り当て可能なキーか
bool is_bindable( int code );

} // namespace blue_sky::key_names
