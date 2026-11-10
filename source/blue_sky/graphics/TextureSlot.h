#pragma once

#include <blue_sky/graphics/ResourceResolver.h>
#include <core/graphics/Texture.h>

namespace blue_sky::graphics
{

/**
 * 描く側（シェーダーなど）が参照し続けるテクスチャ 1 つ
 *
 * GraphicsManager が管理しているテクスチャ（読み込んだ画像）はハンドルで持ち、使うたびに引く。
 * 以前は生ポインタで持ち、テクスチャが解放・差し替えされた後も古いポインタを使い続けて壊れ得た（設計上の問題 B20・D4）。
 *
 * 管理していないテクスチャ（描画先・深度・シャドウマップ・NPR の結果など）は、持ち主が別にいて、描く間は生きている。
 * ハンドルが無いので、それはポインタのまま持つ（設定した側が、描く間生きていることを保証する）。
 */
class TextureSlot
{
public:
	using Texture		= core::graphics::Texture;
	using TextureHandle	= core::graphics::TextureHandle;

private:
	TextureHandle	handle_;				///< 管理しているテクスチャのハンドル
	Texture*		unmanaged_ = nullptr;	///< 管理していないテクスチャ（ハンドルが無いとき）

public:
	TextureSlot() = default;
	TextureSlot( const ResourceResolver& resolver, Texture* t ) { set( resolver, t ); }

	/// テクスチャを設定する（nullptr なら空にする）
	void set( const ResourceResolver& resolver, Texture* t )
	{
		handle_ = t ? resolver.get_texture_handle_of( t ) : TextureHandle();
		unmanaged_ = handle_.is_null() ? t : nullptr;
	}

	/// テクスチャを引く（空・解放済みなら nullptr）
	Texture* get( const ResourceResolver& resolver ) const
	{
		return handle_.is_null() ? unmanaged_ : resolver.resolve_texture( handle_ );
	}

	/// 設定したテクスチャが解放された（ハンドルで持っていて、もう引けない）
	bool is_lost( const ResourceResolver& resolver ) const
	{
		return ! handle_.is_null() && ! resolver.resolve_texture( handle_ );
	}

	/// ハンドルで持っているか（管理しているテクスチャか）
	bool is_managed() const { return ! handle_.is_null(); }
};

} // namespace blue_sky::graphics
