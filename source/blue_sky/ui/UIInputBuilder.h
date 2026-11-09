#pragma once

#include <blue_sky/type.h>
#include <core/ui/NavRepeater.h>
#include <core/ui/UIInput.h>

namespace blue_sky
{
	class Input;
}

namespace blue_sky::ui
{
	class Renderer;

/**
 * Input の生の状態から core::ui::UIInput を作る
 *
 * - メニュー操作は固定キー（矢印・Enter・Esc・Q/E・PageUp/PageDown）とパッドで、ゲームのキー割り当てとは無関係
 * - 方向はキーリピートを適用する
 * - 決定（Enter / パッド A）とクリック（マウス左）は別々に扱う
 * - 最後に使ったデバイス（キーボード・マウス／パッド）を判定する
 */
class UIInputBuilder
{
private:
	core::ui::NavRepeater repeat_[ 4 ];
	float_t last_mouse_x_ = -1.f;
	float_t last_mouse_y_ = -1.f;
	core::ui::InputDevice last_device_ = core::ui::InputDevice::KEYBOARD_MOUSE;

public:
	/// 1 フレーム分の UI 入力を作る（マウスホイールのキューを消費する）
	core::ui::UIInput build( Input& input, const Renderer& renderer, float_t dt );

	/// 状態を捨てる（画面を開いた・最前面に戻ったとき）。マウス位置は現在値で初期化し「動いた」扱いにしない
	void reset( const Input& input, const Renderer& renderer );

}; // class UIInputBuilder

} // namespace blue_sky::ui
