#pragma once

namespace core::graphics
{

/**
 * プリミティブの種類（エンジン独自の型）
 *
 * 以前は D3D11 の値そのものを持つ列挙型の別名だった（設計上の問題 B19）。
 * API の値への変換は実装側（direct_3d_11::to_d3d_primitive_topology）で行う。
 */
enum class PrimitiveTopology
{
	POINT_LIST,
	LINE_LIST,
	TRIANGLE_LIST,
	PATCH_LIST_3_CONTROL_POINT
};

} // namespace core::graphics
