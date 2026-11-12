#pragma once

#include <core/type.h>

namespace core::graphics
{

/**
 * デバッグ表示（線）を描く窓口
 *
 * 線は 1 フレームの間ためておき、描画の実装がフレームの描画でまとめて描く（GraphicsManager::render_debug_bullet）。
 * 物理エンジン（Bullet）のデバッグ表示も同じ線にたまる。
 * 実装は Direct3D 11 の BulletDebugDraw。以前はゲームの側（シーン・Lua の関数）がその実装の型を直接知っていた
 */
class DebugDraw
{
public:
	virtual ~DebugDraw() = default;

	/// 線を 1 本足す（color は r・g・b を x・y・z に入れる）
	virtual void draw_line( const Vector& from, const Vector& to, const Vector& color ) = 0;

	/// 物理エンジンのデバッグ表示の種類を決める（Bullet の btIDebugDraw::DebugDrawModes のビット。0 なら描かない）
	virtual void set_physics_debug_mode( int mode ) = 0;

}; // class DebugDraw

} // namespace core::graphics
