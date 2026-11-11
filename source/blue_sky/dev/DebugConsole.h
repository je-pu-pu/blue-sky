#pragma once

#include <blue_sky/type.h>

#include <functional>

namespace blue_sky
{
	class ScriptManager;
	class SceneManager;
}

namespace blue_sky::dev
{

/**
 * imgui の開発用の窓（Lua のコマンドを打つコンソールと、シーンの一覧）。F1 で開け閉めする
 *
 * 以前は GameMain の中にあった（設計上の問題 D2。doc/GameMainの分割.md）。GameMain の持ち物で、外には見せない
 */
class DebugConsole
{
public:
	/// シーンの一覧でダブルクリックしたシーンを開く（今は切り替えの要求を出さず、すぐにベースを作り直す。以前と同じ）
	using OpenScene = std::function< void( const string_t& name ) >;

private:
	ScriptManager&			script_manager_;
	const SceneManager&		scene_manager_;
	OpenScene				open_scene_;

	bool					is_open_ = false;			///< コマンドモード中か
	string_t				user_command_;				///< ユーザーの入力したコマンド
	bool					scroll_to_bottom_ = false;	///< 次に描くときにログの一番下へ送る
	int						selected_scene_index_ = -1;	///< シーンの一覧で選んでいる行

	void render_console_window();
	void render_scene_list_window();

public:
	DebugConsole( ScriptManager& script_manager, const SceneManager& scene_manager, OpenScene open_scene );

	bool is_open() const { return is_open_; }
	void toggle() { is_open_ = ! is_open_; }

	/// 開いていれば窓を描く（imgui のフレームの中で呼ぶ）
	void render();
};

} // namespace blue_sky::dev
