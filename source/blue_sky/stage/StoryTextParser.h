#pragma once

#include <blue_sky/stage/Action.h>

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

namespace blue_sky::stage
{

/**
 * 値を目標へ少しずつ近づける動き（背景の重なり BgSpriteLayer の chase_value に当てる）
 *
 * 書いていない所は重なりの既定の値のまま（大きさ 1・回転 0・位置 0・色 白、速さ 0）
 */
template< size_t N >
struct ChaseDesc
{
	std::optional< std::array< double, N > >	from;		///< 始めの値
	std::optional< std::array< double, N > >	to;			///< 目標の値
	std::optional< std::array< double, N > >	speed;		///< 1 フレームに近づく量
};

/// 背景の重なり 1 枚
struct StoryLayerDesc
{
	string_t								image;			///< media/image の下の画像のファイル名
	std::optional< std::array< int, 4 > >	src_rect;		///< 画像の切り出し（左・上・右・下。書いていなければ画像全体）
	std::optional< ChaseDesc< 2 > >			translation;	///< 位置（x・y）
	std::optional< ChaseDesc< 1 > >			rotation;		///< 回転（度。左上を中心に回る）
	std::optional< ChaseDesc< 1 > >			scale;			///< 大きさ（倍）
	std::optional< ChaseDesc< 4 > >			color;			///< 色（r・g・b・a。0〜1）
	SourceLocation							where;
};

/// BGM
struct StoryBgmDesc
{
	string_t	file;			///< 音の名前（SoundManager::load_music に渡す）
	double		volume = 1.0;
};

/**
 * ステージの前後の文と背景（.intro.toml・.outro.toml）を読んだ結果（doc/ステージ形式の設計.md 7 章）
 */
struct StoryTextData
{
	std::vector< StoryLayerDesc >		layers;				///< 背景の重なり（書いた順に描く）
	std::optional< string_t >			sound;				///< 音声（鳴り終わるか飛ばすと次へ進む）
	std::optional< StoryBgmDesc >		bgm;
	std::optional< uint32_t >			text_color;			///< 本文の色（0xRRGGBBAA）
	std::optional< uint32_t >			text_border_color;	///< 本文の縁の色（0xRRGGBBAA）
	string_t							text;				///< 本文（UTF-8。空なら本文を流さない）
	std::optional< string_t >			next_scene;			///< 次のシーン（書いていなければシーンの登録で決めたもの）
	std::optional< string_t >			next_stage;			///< 次のステージ

	std::vector< string_t >				warnings;			///< 読めなかった所（"ファイル:行:列: 中身"）
};

/**
 * ステージの前後の文と背景の TOML（.intro.toml・.outro.toml）を読む
 *
 * 失敗の扱い（CLAUDE.md「失敗の扱い」。どれも "ファイル:行:列" 付き）:
 *   - ファイルが無い・文法の誤り・読めない版（format） : 例外（呼ぶ側がログに残し、この場面を飛ばす）
 *   - 知らないキー・型の違い : 警告に残し、そのキーだけ飛ばして続ける（画像の無い重なりは重なりごと飛ばす）
 */
class StoryTextParser
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
	static StoryTextData parse_text( const string_t& text, const string_t& file_name );

	/**
	 * ファイルから読む
	 *
	 * @throw common::exception ファイルが無い・文法の誤り・読めない版
	 */
	static StoryTextData parse_file( const string_t& file_name );
};

} // namespace blue_sky::stage
