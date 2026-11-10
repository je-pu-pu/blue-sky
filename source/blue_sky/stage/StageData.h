#pragma once

#include <blue_sky/stage/Action.h>

#include <array>
#include <optional>
#include <vector>

namespace blue_sky::stage
{

using Vector3d = std::array< double, 3 >;

/// 物体のイベントの受け取り手（スイッチの turn-on など）
struct EventHandlerDesc
{
	string_t	event;		///< イベントの名前（"turn-on"・"turn-off"）
	Action		action;
};

/**
 * ステージに置く物体 1 つ
 *
 * type は ObjectFactory の種類の名前（balloon・switch など）。ほかに次の 3 つがある :
 *   - "static" : 動かない物（model のモデル。今の形式の object・static-object・dynamic-object 行）
 *   - "area-switch" : 空間スイッチ（size の大きさのすり抜ける箱）
 *   - "translation" : 動く床（size の範囲を speed で往復する。今の形式の translation-object 行）
 */
struct ObjectDesc
{
	string_t						type;
	string_t						model;				///< static のモデルの名前
	Vector3d						position = { };
	Vector3d						rotation = { };		///< static の回転（度。x は向きにもなる。以前と同じ）
	double							direction = 0.0;	///< 向き（Y 軸の度）
	Vector3d						size = { };			///< area-switch の大きさ・translation の動く範囲
	double							speed = 0.01;		///< translation の速さ
	std::vector< string_t >			names;				///< 名前（同じ名前は後の物体に付け替わる）
	std::vector< EventHandlerDesc >	events;				///< イベントの受け取り手（書いた順に呼ぶ）
	SourceLocation					where;
};

/// プレイヤーの始めの位置
struct PlayerDesc
{
	Vector3d					position = { };
	std::optional< double >		camera_direction;	///< カメラの上下の向き（度。書いていなければ変えない）
};

/**
 * ステージ 1 つを読んだ結果（ファイルの形式によらない。読む = LegacyStageParser・TomlStageParser、当てる = StageBuilder）
 */
struct StageData
{
	string_t						collision;		///< 当たりの形の .obj（ステージの置き場からの名前。空なら無し）
	string_t						ground;			///< 地面の名前（空なら無し）
	string_t						bgm;			///< 読み込むときに 1 回だけ読む BGM（空なら無し）
	std::optional< double >			bgm_bpm;		///< bgm と一緒に設定する BPM
	std::optional< bool >			bgm_loop;		///< BGM を繰り返すか（書いていなければ繰り返す）

	std::optional< PlayerDesc >		player;
	std::optional< Vector3d >		goal;			///< ゴールの始めの位置

	std::vector< ObjectDesc >		objects;		///< 物体（作る順 = 書いた順）
	std::vector< Action >			start_actions;	///< 始めるたび（やり直すたび）に行う動き（書いた順）

	std::vector< string_t >			warnings;		///< 読んだときの警告（"ファイル:行: 中身"。ログにも出す）
};

/// 読んだ結果を、比べるための文字列にする（場所と警告は含めない。テストで 2 つの読み方の結果を比べる）
string_t describe( const StageData& data );

} // namespace blue_sky::stage
