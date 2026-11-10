#pragma once

#include <core/graphics/Model.h>
#include <core/graphics/Texture.h>

namespace blue_sky::graphics
{

class Model;

/**
 * 描画の資源（モデル・テクスチャ）をハンドル・名前から引くだけの窓口（読み込みも作成もしない）
 *
 * 毎フレーム動く ECS のシステム（ObjectRenderDataSystem・PlayerBehaviorSystem）が使う。const なので、
 * 描く前に資源を作らないことが型で分かる。GraphicsManager が実装する。テストでは偽物を渡す（設計上の問題 D3）
 */
class ResourceResolver
{
public:
	virtual ~ResourceResolver() = default;

	/// モデルのハンドルからモデルを引く（解放済み・空なら nullptr）
	virtual Model* resolve_model( core::graphics::ModelHandle ) const = 0;

	/// 管理しているモデルのハンドル（管理していなければ空のハンドル）
	virtual core::graphics::ModelHandle get_model_handle( const core::graphics::Model* ) const = 0;

	/// テクスチャのハンドルからテクスチャを引く（解放済み・空なら nullptr）
	virtual core::graphics::Texture* resolve_texture( core::graphics::TextureHandle ) const = 0;

	/// 名前のテクスチャのハンドル（無ければ空のハンドル）
	virtual core::graphics::TextureHandle get_texture_handle( const char_t* name ) const = 0;

	/// 管理しているテクスチャのハンドル（管理していない（描画先など）・nullptr なら空のハンドル）
	virtual core::graphics::TextureHandle get_texture_handle_of( const core::graphics::Texture* ) const = 0;
};

} // namespace blue_sky::graphics
