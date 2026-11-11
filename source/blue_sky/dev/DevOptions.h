#pragma once

#include <blue_sky/type.h>
#include <Scene/SceneRequest.h>

#include <functional>
#include <optional>
#include <vector>

namespace blue_sky::dev
{

/**
 * 開発用の環境変数の値（GameMain が起動時に 1 回だけ読む）
 *
 * 以前は GameMain の各所（コンストラクタ・setup_scene・フレームの保存など）がその場で環境変数を読んでいた。
 * 名前と意味は変えていない。どれも設定しなければ何もしない。
 * シーンが読む開発用の環境変数（BLUE_SKY_DEBUG_DUMP_STAGE など）はシーンのもので、ここには入れない
 */
struct DevOptions
{
	/// 環境変数を読む関数（無ければ nullopt。値が空でも設定されていれば空の文字列）。テストでは偽物を渡す
	using GetEnvironment = std::function< std::optional< string_t >( const char_t* name ) >;

	std::optional< string_t >	start_scene;				///< BLUE_SKY_START_SCENE : 起動時のシーン（設定 debug.start_scene より優先）
	std::optional< string_t >	start_stage;				///< BLUE_SKY_START_STAGE : 起動時のステージ（game_play はステージ名が要る）
	std::optional< string_t >	start_overlay;				///< BLUE_SKY_START_OVERLAY : 起動時に開くオーバーレイ（設定 debug.start_overlay より優先。"options?tab=controls" のように引数も付けられる）

	std::vector< int >			capture_frames;				///< BLUE_SKY_CAPTURE_FRAMES="60,120" : 描画結果を PNG に保存するフレーム
	string_t					capture_directory = "dump";	///< BLUE_SKY_CAPTURE_DIR : 保存先（既定 dump）
	bool						capture_glyph_atlas = false;	///< BLUE_SKY_CAPTURE_GLYPH_ATLAS : 文字のアトラスも保存する

	int							display_change_width = 0;	///< BLUE_SKY_DEBUG_DISPLAY_CHANGE="幅x高さ" : 120 フレーム目に解像度を変えて適用する（0 なら変えない）
	int							display_change_height = 0;

	unsigned long				fixed_step_msec = 0;		///< BLUE_SKY_DEBUG_FIXED_STEP_MSEC : 1 フレームの経過時間をいつもこの値にする（0 なら本当の時間）

	int							frame_time_first = 0;		///< BLUE_SKY_DEBUG_FRAME_TIME="最初,最後" : フレームの時間を測る範囲（0 なら測らない）
	int							frame_time_last = 0;

	std::vector< SceneRequest >	scene_sequence;				///< BLUE_SKY_DEBUG_SCENE_SEQUENCE="stage_select,game_play:1-1,title" : 順に置き換えるベースシーン

	/// 環境変数を読んで値を作る（読めない・形の違う値は、設定しなかったのと同じにする）
	static DevOptions read( const GetEnvironment& get_environment );

	/// このプロセスの環境変数から読む
	static DevOptions read_from_environment();
};

/// このプロセスの環境変数を読む（無ければ nullopt）
std::optional< string_t > get_environment_variable( const char_t* name );

} // namespace blue_sky::dev
