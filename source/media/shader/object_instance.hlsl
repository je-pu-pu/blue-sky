/**
 * 同じモデルの物体をまとめて 1 回で描く（インスタンス描画）ための、物体ごとの定数
 *
 * 1 個ずつ描くときの ObjectConstantBuffer（b2）と同じ中身を、物体ごとに構造化バッファ（t11）に並べる
 * （C++ の ObjectConstantBufferData と同じ並び。core::graphics::ObjectInstanceBuffer が書く）。
 * まとめごとの物体の番号の表（t12）の ObjectInstanceOffset 番目から、インスタンスの番号（SV_InstanceID）だけ進めた所に、
 * 描く物体の番号がある。
 *
 * まとめて描くシェーダーは、1 個ずつ描くシェーダーと同じ計算を、World・ObjectColor の代わりにここから読んだ値で行う
 * （見た目を変えないため。計算は共通の関数にして両方から呼ぶ）
 */
struct ObjectInstanceData
{
	row_major matrix World;
	float4 Color;
	row_major matrix PrevWorld;
};

StructuredBuffer< ObjectInstanceData > ObjectInstances : register( t11 );
StructuredBuffer< uint > ObjectInstanceIndices : register( t12 );

cbuffer ObjectInstanceConstantBuffer : register( b6 )
{
	uint ObjectInstanceOffset;		// このまとめの先頭の、番号の表の中の位置
};

/**
 * まとめの中の instance_id 番目の物体の定数を返す
 */
ObjectInstanceData get_object_instance( uint instance_id )
{
	return ObjectInstances[ ObjectInstanceIndices[ ObjectInstanceOffset + instance_id ] ];
}
