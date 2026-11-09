#pragma once

#include <core/graphics/Direct3D11/DynamicPointList.h>

namespace core::graphics
{

/**
 * 点の列を動的に描く（抽象層の名前。実装は Direct3D 11）
 *
 * 以前は Direct3D 11 の実装そのものが抽象層のフォルダに置かれていた（設計上の問題 B19）。
 */
template< typename VertexType >
using DynamicPointList = direct_3d_11::DynamicPointList< VertexType >;

} // namespace core::graphics
