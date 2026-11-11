#pragma once

#include <core/ConstantBuffers.h>
#include <core/type.h>

#include <cstdint>

namespace core::graphics
{

/**
 * 同じモデルの物体をまとめて 1 回で描く（インスタンス描画）ための、物体ごとの定数の入れ物
 *
 * 1 個ずつ描くときの物体の定数バッファ（ObjectConstantBufferData : world・色・前フレームの world）と同じ中身を、
 * 物体ごとに 1 つずつ並べた表（update_instances。1 フレームに 1 回）と、まとめごとに「どの物体を描くか」の番号の表
 * （update_indices。描く段ごと）を持つ。シェーダーは番号の表の offset 番目から、インスタンスの番号の分だけ進めた所の
 * 物体の定数を読む（media/shader/object_instance.hlsl）。
 *
 * 以前からある InstanceBuffer（world 行列だけ・1,024 個まで。core の RenderSystem が使う）とは別物
 */
class ObjectInstanceBuffer
{
public:
	using Data = ObjectConstantBufferData;
	using Index = std::uint32_t;

	virtual ~ObjectInstanceBuffer() = default;

	/// 物体ごとの定数の表を書く（足りなければ大きく作り直す。count が 0 なら何もしない）
	virtual void update_instances( const Data* data, size_t count ) = 0;

	/// まとめごとの物体の番号の表を書く（足りなければ大きく作り直す。count が 0 なら何もしない）
	virtual void update_indices( const Index* indices, size_t count ) = 0;

	/**
	 * 頂点シェーダーに結ぶ（次に描くまとめが、番号の表の offset 番目から始まることも渡す）
	 *
	 * @param offset まとめの先頭の、番号の表の中の位置
	 */
	virtual void bind_to_vs( Index offset ) const = 0;

}; // class ObjectInstanceBuffer

} // namespace core::graphics
