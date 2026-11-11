#pragma once

namespace core::graphics
{

/**
 * ピクセルフォーマット（エンジン独自の型）
 *
 * 以前は DXGI_FORMAT の値そのものを持つ Direct3D11 の列挙型の別名で、抽象層を使うだけで d3d11.h が必要だった
 * （設計上の問題 B19）。API の値への変換は実装側（direct_3d_11::to_dxgi_format）で行う。
 */
enum class PixelFormat
{
	R8G8B8A8_UNORM,
	R8_UINT,
	R32G32_FLOAT,		///< モーションベクトル (速度) G-buffer 用
};

} // namespace core::graphics
