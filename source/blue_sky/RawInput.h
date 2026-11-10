#pragma once

#include <array>
#include <vector>

#include <windows.h>

namespace blue_sky::raw_input
{

/**
 * 入力の出来事（そのフレームに起きたこと）
 *
 * Raw Input（WM_INPUT）から作る。以前は入力を毎フレームのポーリング（GetAsyncKeyState と DirectInput）だけで読み、
 * フレームの間に起きたことの順番や移動量の取りこぼしを、ウィンドウメッセージの押下の記録で補っていた（設計上の問題 B27）。
 */
struct Event
{
	enum class Type
	{
		KEY_DOWN,		///< キー・マウスボタンを押した（code は仮想キー。押し続けによる繰り返しは含まない）
		KEY_UP,			///< 離した
		MOUSE_MOVE,		///< マウスが動いた（dx, dy は相対の移動量）
		MOUSE_WHEEL,	///< ホイール（dy が回転量。120 が 1 段）
	};

	Type type = Type::KEY_DOWN;
	int code = 0;
	long dx = 0;
	long dy = 0;
};

/**
 * RAWINPUT を出来事の列にする
 *
 * Windows の構造体だけを使い、ウィンドウやデバイスは要らないので、テストで RAWINPUT を作って確かめられる。
 * キーの押し続けによる繰り返し（Raw Input は押している間 make を送り続ける）は、押している状態を覚えて取り除く。
 */
class Decoder
{
private:
	std::array< bool, 256 > key_down_ {};

public:
	void decode( const RAWINPUT& raw, std::vector< Event >& out );

	/// 押している状態を忘れる（フォーカスを失ったとき。離したことを受け取れないため）
	void reset() { key_down_.fill( false ); }
};

/// マウスとキーボードの Raw Input をウィンドウに登録する（失敗したら false。呼び出し側は以前のやり方を使う）
bool register_devices( HWND hwnd );

/// WM_INPUT の lParam から RAWINPUT を読み、出来事にする（読めなければ false）
bool read( LPARAM lp, Decoder& decoder, std::vector< Event >& out );

} // namespace blue_sky::raw_input
