#pragma once

#include <core/type.h>

#include <initializer_list>
#include <unordered_map>

namespace core::ui
{

/**
 * 表示文字列テーブル
 *
 * UI に表示する文字列をキー経由で引く。未登録のキーはキー文字列そのものを返すため、
 * 翻訳が欠けていても画面が空になることはない（どのキーが欠けているかが見える）。
 *
 * ファイル形式（UTF-8）:
 * @code
 * # コメント
 * options.title = オプション
 * dialog.keep   = この設定を維持しますか？（残り {0} 秒）
 * @endcode
 *
 * - 先頭・末尾の空白は無視する。`=` の左がキー、右が値。
 * - 値の中の `\n` は改行に変換する。
 * - UTF-8 BOM は読み飛ばす。
 */
class StringTable
{
public:
	using Map = std::unordered_map< string_t, string_t >;

private:
	Map table_;

public:
	/// 文字列からまとめて読み込む（既存のキーは上書き）。不正な行は無視し、読めた件数を返す
	int load_from_string( const string_t& text );

	/// ファイルから読み込む。開けなければ false
	bool load_file( const char_t* path );

	/// 1 件登録する
	void set( const string_t& key, const string_t& value ) { table_[ key ] = value; }

	/// キーが登録済みか
	bool has( const string_t& key ) const { return table_.find( key ) != table_.end(); }

	/// キーに対応する文字列。未登録ならキーそのものを返す
	const string_t& get( const string_t& key ) const;

	/// `{0}` `{1}` … を引数で置換した文字列を返す
	string_t format( const string_t& key, std::initializer_list< string_t > args ) const;

	/// 任意の書式文字列の `{0}` `{1}` … を置換する
	static string_t format_text( const string_t& text, std::initializer_list< string_t > args );

	size_t size() const { return table_.size(); }
	void clear() { table_.clear(); }

}; // class StringTable

} // namespace core::ui
