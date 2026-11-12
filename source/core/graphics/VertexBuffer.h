#pragma once

#include <cstddef>

namespace core::graphics
{

/**
 * CPU から書き換えられる頂点の置き場
 *
 * 作るのは GraphicsManager::create_dynamic_vertex_buffer（大きさは作るときに決める）。
 * 毎フレーム中身を書き換えて描くもの（粒子・点の列など）に使う。
 * 以前は Direct3D 11 の実装のテンプレートの別名で、使う側（ECS のシステム・シーン）が Direct3D 11 を受け取って作っていた
 */
class VertexBuffer
{
public:
	VertexBuffer() = default;
	virtual ~VertexBuffer() = default;
	VertexBuffer( const VertexBuffer& ) = delete;
	VertexBuffer& operator = ( const VertexBuffer& ) = delete;

	/**
	 * 中身を書き換える（前の中身は捨てる）
	 *
	 * @param vertices 頂点の並び
	 * @param vertex_count 頂点の数（作ったときの数まで）
	 */
	virtual void update( const void* vertices, size_t vertex_count ) = 0;

	/**
	 * 入力に置き、先頭から vertex_count 個の頂点を描く
	 *
	 * 形（点・線など）・入力レイアウト・シェーダーは、呼ぶ側が先に決めておく
	 */
	virtual void draw( size_t vertex_count ) const = 0;

}; // class VertexBuffer

} // namespace core::graphics
