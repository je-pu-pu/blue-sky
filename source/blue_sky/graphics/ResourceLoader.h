#pragma once

#include <blue_sky/graphics/ResourceResolver.h>
#include <core/graphics/ConstantBuffer.h>

#include <memory>

namespace blue_sky::graphics
{

/**
 * 描画の資源を読み込み・作る窓口（引く窓口 ResourceResolver に、読み込み・複製・定数バッファの作成を足したもの）
 *
 * 物体を作る所（ObjectFactory・GameObjectRef・RobotBehaviorSystem の見た目のテクスチャ）が使う。
 * GraphicsManager が実装する。テストでは偽物を渡し、何を読み・作ったかを確かめる（設計上の問題 D3）
 */
class ResourceLoader : public ResourceResolver
{
public:
	/// 名前のモデルを読む（読み込み済みならそれを返す。読めなければ nullptr）
	virtual Model* load_model( const char_t* name ) = 0;

	/// モデルとシェーダーを複製する（メッシュ・線・アニメーションは共有する。1 つの物体だけ見た目を変えるとき）
	virtual Model* clone_model_with_shaders( const Model* ) = 0;

	/// 名前のテクスチャ（無ければ media/model/<名前>.png を読んで登録する。読めなければ nullptr）
	virtual core::graphics::Texture* get_texture( const char_t* name ) = 0;

	/// 定数バッファを作る（物体ごとの world・ボーン行列など）
	virtual std::unique_ptr< core::graphics::ConstantBuffer > create_constant_buffer( size_t size, int slot, core::graphics::ConstantBufferUsage usage ) = 0;
};

} // namespace blue_sky::graphics
