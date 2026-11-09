#pragma once

#include <core/type.h>

namespace core::ui
{

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
