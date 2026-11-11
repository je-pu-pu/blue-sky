#pragma once

#include <blue_sky/dev/DevOptions.h>

#include <deque>
#include <functional>
#include <vector>

namespace game { class MainLoop; }

namespace blue_sky::dev
{

/**
 * 開発用の機能（どれも開発用の環境変数を設定したときだけ動く。DevOptions）
 *
 * - フレームの保存（BLUE_SKY_CAPTURE_FRAMES・CAPTURE_DIR・CAPTURE_GLYPH_ATLAS）
 * - フレームの時間の記録（BLUE_SKY_DEBUG_FRAME_TIME）
 * - シーンを順に替える（BLUE_SKY_DEBUG_SCENE_SEQUENCE）
 * - 解像度の変更の試し（BLUE_SKY_DEBUG_DISPLAY_CHANGE）
 * - 経過時間の固定（BLUE_SKY_DEBUG_FIXED_STEP_MSEC）
 * - ゲーム中のスクショ（F11。これだけは環境変数なしで動く。開発用の窓を写さない）
 *
 * 以前は GameMain の中にあった（設計上の問題 D2。doc/GameMainの分割.md）。GameMain の持ち物で、外には見せない。
 * 画面の保存・設定の変更・シーンの切り替えなどの GameMain の仕事は Hooks で受け取る（テストでは偽物を渡す）
 */
class DevTools
{
public:
	struct Hooks
	{
		std::function< bool( const std::wstring& path ) >		save_back_buffer;		///< 描画結果を PNG に保存する
		std::function< void( const std::wstring& path ) >		save_glyph_atlas;		///< 文字のアトラスを PNG に保存する
		std::function< void( int width, int height ) >			change_display;			///< 解像度を変えて適用する
		std::function< void( const SceneRequest& ) >			request_scene;			///< シーンの切り替えを要求する
		std::function< string_t() >								current_scene_name;		///< 今のベースシーンの名前（無ければ空）
	};

	/// 解像度の変更の試しをするフレーム
	static constexpr int DISPLAY_CHANGE_FRAME = 120;

	/// シーンを順に替える間隔（フレーム）
	static constexpr int SCENE_SEQUENCE_INTERVAL = 150;

private:
	const DevOptions&				options_;
	Hooks							hooks_;

	std::vector< int >				capture_frames_;
	string_t						capture_directory_;
	int								frame_count_ = 0;		///< 描いたフレームの数（保存するファイルの番号）

	int								frame_time_first_ = 0;
	int								frame_time_last_ = 0;
	long long						frame_time_start_ = 0;				///< このフレームの update の始まり（QueryPerformanceCounter）
	long long						frame_time_previous_start_ = 0;		///< 前のフレームの update の始まり
	std::vector< double >			frame_time_work_ms_;				///< update の始まりから present の前まで（ms）
	std::vector< double >			frame_time_interval_ms_;			///< 前のフレームの始まりからの間隔（ms）
	std::vector< double >			frame_time_present_ms_;				///< present にかかった時間（ms。GPU が追いつかないと、ここで待つ）

	std::deque< SceneRequest >		scene_sequence_;					///< 順に置き換えるベースシーン
	int								scene_sequence_frames_ = 0;			///< 前の置き換えからのフレーム数

	bool							screenshot_requested_ = false;		///< ゲーム中のスクショを頼まれた（F11）
	int								screenshot_number_ = 0;				///< 次に試すスクショの番号（あるファイルは飛ばす）
	string_t						screenshot_directory_ = SCREENSHOT_DIRECTORY;

	void capture_frame_if_requested();
	void record_frame_time();

public:
	DevTools( const DevOptions& options, Hooks hooks );

	/// 経過時間の固定を MainLoop に設定する（BLUE_SKY_DEBUG_FIXED_STEP_MSEC。GameMain が MainLoop を作った直後に呼ぶ）
	void apply_fixed_step( game::MainLoop& main_loop ) const;

	/// フレームの保存・フレームの時間・シーンを順に替える準備をする（起動のシーンを作った後に呼ぶ）
	void setup();

	/// update の始まりに呼ぶ（フレームの時間を測る）
	void begin_frame();

	/// 起動時のオーバーレイの後、シーンの切り替えの前に呼ぶ。決まった間隔で次のシーンへの置き換えを要求する
	void update_scene_sequence( bool is_loading_frame );

	/// 描き終えて present する前に呼ぶ（フレームを数え、解像度の変更の試し・フレームの保存・フレームの時間の記録をする）
	void after_render();

	/// ゲーム中のスクショを 1 枚頼む（F11。環境変数なしで使える。次の capture_screenshot_if_requested で保存する）
	void request_screenshot() { screenshot_requested_ = true; }

	/// スクショを頼まれていれば、imgui を描く前の画面を SCREENSHOT_DIRECTORY/shot_%04d.png に保存する
	/// （ImGui::Render の前に呼ぶので、開発用の窓が写らない。あるファイルは上書きせず次の番号にする）
	void capture_screenshot_if_requested();

	/// ゲーム中のスクショの保存先を替える（テストで一時フォルダにする）
	void set_screenshot_directory( const string_t& directory ) { screenshot_directory_ = directory; }

	/// ゲーム中のスクショの既定の保存先（作業ディレクトリから。.gitignore 済み）
	static constexpr const char_t* SCREENSHOT_DIRECTORY = "screenshot";

	/// present を呼び、測る範囲ならかかった時間を記録する
	void present( const std::function< void() >& present );

	int get_frame_count() const { return frame_count_; }
	bool is_measuring_frame_time() const { return frame_time_last_ > 0; }
};

} // namespace blue_sky::dev
