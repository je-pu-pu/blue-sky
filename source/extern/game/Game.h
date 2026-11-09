#pragma once

#include <type/type.h>

namespace game
{

/**
 * 抽象ゲームクラス
 *
 */
class Game
{
public:

protected:
	Game() { }

public:
	static Game* get_instance();

	virtual ~Game() { }

	virtual bool update() = 0;

	virtual void on_key_down( char_t ) =0;
	virtual void on_special_key_down( int ) = 0;
	virtual void on_mouse_wheel( int ) = 0;
	virtual void on_resize() = 0;

	/// フルスクリーンの切り替えの要求（Alt+Enter）
	virtual void on_toggle_full_screen() { }

	/// アプリケーションが再びアクティブになった
	virtual void on_activated() { }

	/// 正常終了の後片付け（設定の保存など）。メッセージループが正常に終わったときだけ呼ばれる
	virtual void shutdown() { }

	/// update() が次に処理を行うまでの待ち時間（ミリ秒）。メッセージループがこの間眠る
	virtual unsigned long get_idle_wait_msec() const { return 0; }

	virtual int get_width() const = 0;
	virtual int get_height() const = 0;

	virtual bool is_show_cursor() const = 0;
};

} // namespace game
