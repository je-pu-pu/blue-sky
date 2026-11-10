#pragma once

#include <blue_sky/stage/StageData.h>

#include <iosfwd>

namespace blue_sky::stage
{

/**
 * 今の独自の形式（.stage。1 行 1 コマンド・空白区切り）を読んで StageData にする
 *
 * 以前は GamePlayScene::load_stage_file が読みながらゲームに当てていた。読む所と当てる所（StageBuilder）を分け、
 * 読み方の決まり（足りない数は 0・余った数は捨てる・name は 1 語目だけ など）は以前と同じにした。
 * 以前は黙って捨てていたもの（余った数・どこも読まない行）は警告にする。
 */
class LegacyStageParser
{
public:
	/**
	 * 読む
	 *
	 * @param in 読むもの
	 * @param file_name ファイル名（警告と例外に付ける）
	 * @throw common::exception 物体の無い所の name・event 行（以前と同じ）
	 */
	static StageData parse( std::istream& in, const string_t& file_name );

	/// 文字列から読む（テスト）
	static StageData parse_text( const string_t& text, const string_t& file_name );
};

} // namespace blue_sky::stage
