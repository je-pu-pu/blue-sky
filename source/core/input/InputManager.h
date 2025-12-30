#pragma once

namespace core::input
{

/**
 * ボタンの定義
 */
enum class Button
{
	LEFT,
	RIGHT,
	UP,
	DOWN,
	A,
	B,
	JUMP,
	L,
	R,
	L2,
	R2,
	ESCAPE,
	MAX,
	NONE
};

/**
 * 入力管理の抽象基底クラス
 */
class InputManager
{
public:
	virtual ~InputManager() = default;

	/**
	 * 現在ボタンが押されているかどうかを取得する
	 */
	virtual bool press( Button button ) const = 0;

	/**
	 * たった今ボタンが押されたかどうかを取得する
	 */
	virtual bool push( Button button ) const = 0;

	/**
	 * たった今ボタンが離されたかどうかを取得する
	 */
	virtual bool release( Button button ) const = 0;

	/**
	 * マウスの X 移動量を取得する
	 */
	virtual float get_mouse_dx() const = 0;

	/**
	 * マウスの Y 移動量を取得する
	 */
	virtual float get_mouse_dy() const = 0;

	/**
	 * マウスの移動量をクリアする
	 */
	virtual void clear_mouse_move() = 0;

	/**
	 * マウスの X 座標を取得する
	 */
	virtual int get_mouse_x() const = 0;

	/**
	 * マウスの Y 座標を取得する
	 */
	virtual int get_mouse_y() const = 0;

}; // class InputManager

} // namespace core::input
