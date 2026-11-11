#pragma once

#include <blue_sky/stage/StageData.h>

namespace blue_sky::stage
{

/**
 * ステージの TOML（.stage.toml）を読んで StageData にする（書き方は doc/ステージ形式の設計.md 2.2）
 *
 * 失敗の扱い（設計 3.4。どれも "ファイル:行:列" 付き）:
 *   - ファイルが無い・文法の誤り・読めない版（format） : 例外（ステージを始められない）
 *   - 知らない種類・知らない動き・知らないキー・型の違い : 警告に残し、その項目（物体・動き・キー）だけ飛ばして続ける
 *     （知らないキーはそのキーだけ、型の違いと必須の欠けはその物体・動きを飛ばす）
 *   - 名前の参照が見つからない : 読み終わったときにまとめて警告に出す（動きは残す。遊んでいる最中は何もしない）
 *   - 同じ名前が 2 つ : 警告に残し、後の物体に付け替える（以前と同じ）
 * 警告は StageData::warnings に入れる（StageBuilder がログに出す）
 * [intro]・[outro]（ステージの前後の文と背景）は読まない（StoryTextParser が読む）
 */
class TomlStageParser
{
public:
	/// 今の形式の版（format の値）
	static constexpr int FORMAT_VERSION = 1;

	/**
	 * 文字列から読む
	 *
	 * @param text TOML
	 * @param file_name ファイル名（警告と例外に付ける）
	 * @throw common::exception 文法の誤り・読めない版
	 */
	static StageData parse_text( const string_t& text, const string_t& file_name );

	/**
	 * ファイルから読む
	 *
	 * @throw common::exception ファイルが無い・文法の誤り・読めない版
	 */
	static StageData parse_file( const string_t& file_name );
};

} // namespace blue_sky::stage
