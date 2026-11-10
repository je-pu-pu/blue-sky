#pragma once

#include "Widget.h"

namespace core::ui
{

/**
 * クリック判定（押して、同じ部品の上で離したら確定）
 *
 * 部品は on_pointer に来た出来事をそのまま feed() に渡し、true が返ったらクリックの処理をする。
 * 押している間の見た目には is_pressed() を使う。捕捉が外れたとき（on_capture_lost）は cancel() を呼ぶ。
 *
 * - PRESS で押下中になる
 * - RELEASE で押下中を解き、押下中だった かつ 部品の矩形内で離したときだけ true を返す
 * - DRAG は何も変えない
 */
class ClickTracker
{
private:
	bool pressed_ = false;

public:
	/**
	 * ポインタの出来事を 1 つ受け取る
	 *
	 * @return このときクリックが確定したら true
	 */
	bool feed( const PointerEvent& e )
	{
		switch ( e.type )
		{
			case PointerEvent::Type::PRESS:
				pressed_ = true;
				return false;

			case PointerEvent::Type::RELEASE:
			{
				const bool was_pressed = pressed_;
				pressed_ = false;
				return was_pressed && e.inside;
			}

			default:
				return false;
		}
	}

	/// 押下中か（押した見た目に使う）
	bool is_pressed() const { return pressed_; }

	/// 押下中を取り消す（捕捉が外れたとき。次の RELEASE はクリックにならない）
	void cancel() { pressed_ = false; }
};

} // namespace core::ui
