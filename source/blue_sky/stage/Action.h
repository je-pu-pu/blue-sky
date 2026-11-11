#pragma once

#include <type/type.h>

#include <map>
#include <optional>
#include <variant>
#include <vector>

namespace blue_sky::stage
{

/// ステージのファイルの中の場所（ログに出す。読んだ結果を比べるときは見ない）
struct SourceLocation
{
	string_t	file;
	int			line = 0;		///< 1 から（0 は分からない）
	int			column = 0;		///< 1 から（0 は分からない）

	/// "ファイル:行:列"（分からない所は省く）
	string_t to_string() const;
};

/// 動きの種類（doc/ステージ形式の設計.md 2.2 の「動き」。TOML の do の値）
enum class ActionType
{
	SET,					///< 環境を変える（sky_box・bgm・bpm・色・光の位置・線・紙 など）
	FADE,					///< 色・光の位置を少しずつ変える
	PLAY_SOUND,
	STOP_SOUND,
	STOP_ALL_SOUNDS,
	LOAD_SOUND,
	SPAWN,					///< 動かない物を作る
	MOVE_TO,				///< 目標地点へ動かす
	TURN_TO,				///< 決まった向き・物体の方を向かせる
	SET_BODY,				///< 剛体の質量・重力・回転のしやすさ・キネマティック・当たりの応答
	SET_FLICKER,			///< ゆらぎの大きさ
	START_FLICKERING,		///< プレイヤーのゆらぎを始める
	PLAY_ANIMATION,
	SET_ANIMATION_SPEED,
	SWITCH,					///< スイッチを 入・切・壊れた にする（イベントは起こさない）
	SET_MODE,				///< 少女・敵の動き方（float・rotation・patrol）
	SET_VISIBLE,			///< 本体・線を描くか
	BREAK_ANIMATION,		///< アニメーションを壊す
	END_STAGE,				///< ステージを終える（最後のスイッチ）
};

/// 動きの名前（TOML の do の値）
const char* to_string( ActionType type );

/// 名前から動きの種類を引く（知らなければ std::nullopt）
std::optional< ActionType > find_action_type( const string_t& name );

/// 引数の値（整数も数として持つ。どの型かは引数の決まりで決まる）
using ActionValue = std::variant< bool, double, string_t, std::vector< double > >;

/**
 * 動き 1 つ（種類と型の付いた引数）
 *
 * 読むとき（TomlStageParser）に作り、遊んでいる最中に文字列を解釈し直さない。
 * 書いていない引数は、行う側（GamePlayScene::run_action）が今までと同じ既定の値を使う
 */
struct Action
{
	ActionType							type = ActionType::SET;
	std::map< string_t, ActionValue >	args;				///< 引数（キーの順）

	bool								is_timed = false;	///< 時間を置いて行うか（delay・every）
	double								interval = 0.0;		///< 何秒ごとに行うか（delay なら その秒数）
	int									count = 1;			///< 何回行うか（delay なら 1）

	SourceLocation						where;

	bool has( const string_t& key ) const { return args.find( key ) != args.end(); }

	/// 数（無い・型が違えば既定の値）
	double number( const string_t& key, double default_value ) const;

	/// 真偽（無い・型が違えば既定の値）
	bool flag( const string_t& key, bool default_value ) const;

	/// 文字列（無い・型が違えば空）
	const string_t& text( const string_t& key ) const;

	/// 数の並び（無い・型が違えば nullptr）
	const std::vector< double >* numbers( const string_t& key ) const;
};

/// 引数の型
enum class ArgKind
{
	BOOL,
	INT,		///< 整数（TOML の整数だけ）
	NUMBER,		///< 数（整数でも小数でもよい）
	STRING,
	NUMBERS,	///< 数の並び（min_count 個から max_count 個）
};

/// 引数 1 つの決まり
struct ArgSpec
{
	const char*	key;
	ArgKind		kind;
	size_t		min_count = 0;		///< NUMBERS の最少の個数
	size_t		max_count = 0;		///< NUMBERS の最多の個数
	bool		required = false;
};

/// 動きの種類ごとの引数の決まり（読むときに型を調べる）
const std::vector< ArgSpec >& get_arg_specs( ActionType type );

/// 引数のキーの決まり（知らないキーなら nullptr）
const ArgSpec* find_arg_spec( ActionType type, const string_t& key );

/// 物体の名前を指す引数（読み終わったときに名前があるかを調べる）
inline bool is_reference_key( const string_t& key ) { return key == "target" || key == "face"; }

/**
 * 引数の組み合わせを調べる（型は読む側が調べた後。必須・どちらか一方・値の範囲）
 *
 * @return 問題（無ければ空）
 */
string_t validate_action( const Action& action );

/// 比べるための文字列（場所は含めない）
string_t describe( const Action& action );

} // namespace blue_sky::stage
