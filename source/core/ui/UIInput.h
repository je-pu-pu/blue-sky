#pragma once

#include <core/type.h>

#include <string>
#include <vector>

namespace core::ui
{

/**
 * UI の操作（アクション）の ID
 *
 * 部品は Widget::on_action() で受け取り、使わなければ親へ回る。
 * 組み込みのアクションは ui_action の定数。ゲーム側の独自の操作（「この行を既定値に戻す」など）は
 * ui_action::USER 以上の ID を使い、Screen::register_action() で操作ヒントの名前を登録する。
 */
using ActionId = int;

namespace ui_action
{
	inline constexpr ActionId NONE		= 0;
	inline constexpr ActionId NAV_UP	= 1;
	inline constexpr ActionId NAV_DOWN	= 2;
	inline constexpr ActionId NAV_LEFT	= 3;
	inline constexpr ActionId NAV_RIGHT	= 4;
	inline constexpr ActionId CONFIRM	= 5;	///< 決定
	inline constexpr ActionId CANCEL	= 6;	///< キャンセル・戻る
	inline constexpr ActionId TAB_PREV	= 7;	///< 前のタブ
	inline constexpr ActionId TAB_NEXT	= 8;	///< 次のタブ

	// 操作ヒントの表示だけに使う（入力としては届かない）
	inline constexpr ActionId HINT_CHANGE	= 20;	///< 左右で変更
	inline constexpr ActionId HINT_TAB		= 21;	///< タブ切替

	inline constexpr ActionId USER		= 100;	///< ゲーム側の独自の操作はこれ以上
}

/// 最後に操作に使われた入力デバイス（操作ヒントの表記切替に使う）
enum class InputDevice
{
	KEYBOARD_MOUSE,
	GAMEPAD
};

/// 方向
enum class NavDir
{
	UP,
	DOWN,
	LEFT,
	RIGHT
};

/**
 * UI 用の 1 フレーム分の入力
 *
 * プラットフォームの入力 API から翻訳済みの値型。ゲームのキー割り当てとは無関係
 * （メニューは常に矢印・Enter・Esc・パッドで操作できる）。
 * テストでは直接組み立てて Screen に渡す。
 */
struct UIInput
{
	// 方向（キーリピート適用済み：押した瞬間と、長押し中の一定間隔で true）
	bool up = false;
	bool down = false;
	bool left = false;
	bool right = false;

	bool confirm = false;		///< 決定（Enter / パッド A）。マウスのクリックは含まない
	bool cancel = false;		///< キャンセル（Esc / パッド B）
	bool tab_prev = false;		///< 前のタブ（Q / PageUp / LB）
	bool tab_next = false;		///< 次のタブ（E / PageDown / RB）

	// ポインタ（仮想座標。画面外では範囲外の値になる）
	float_t pointer_x = -1.f;
	float_t pointer_y = -1.f;
	bool pointer_moved = false;
	bool pointer_pressed = false;	///< 左ボタンを押した瞬間
	bool pointer_down = false;		///< 左ボタンを押している間
	bool pointer_released = false;	///< 左ボタンを離した瞬間
	int wheel = 0;					///< ホイールのノッチ数（+ 奥 / - 手前）

	int pushed_key = 0;				///< このフレームに押された割り当て可能キー（0 = なし。キー割り当て用）

	/// ゲーム側の独自の操作（ui_action::USER 以上）。このフレームに起きた順
	std::vector< ActionId > actions;

	/**
	 * このフレームに入力された文字（確定した文字。IME の変換確定後の文字を含む）
	 *
	 * 文字を受け取る部品（TextField）がフォーカス中なら、文字はそちらへ渡り、
	 * 同じキーによるタブ切替（Q / E）は行わない。U+0008 はバックスペース。
	 */
	std::u32string text;

	InputDevice last_device = InputDevice::KEYBOARD_MOUSE;

	/// 方向入力を NavDir で取り出す
	bool nav( NavDir d ) const
	{
		switch ( d )
		{
			case NavDir::UP: return up;
			case NavDir::DOWN: return down;
			case NavDir::LEFT: return left;
			default: return right;
		}
	}

}; // struct UIInput

} // namespace core::ui
