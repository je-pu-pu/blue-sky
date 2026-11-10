#pragma once

#include "ShaderResource.h"
#include <core/ResourceHandle.h>

namespace core::graphics
{

class Texture;

/// テクスチャを指すハンドル（引くときは blue_sky の GraphicsManager::resolve_texture）
using TextureHandle = core::ResourceHandle< Texture >;

/**
 * 抽象テクスチャクラス
 *
 * 実際のテクスチャメモリを確保し、シェーダーリソースとして使用できる
 */
class Texture : public ShaderResource
{
public:
	Texture() { }
	virtual ~Texture() { }

	virtual uint_t get_width() const = 0;
	virtual uint_t get_height() const = 0;

	virtual uint_t get_multi_sample_count() const = 0;

	/**
	 * 乗算済みアルファ（RGB×A）の画素を持つか
	 *
	 * テクスチャ自身が持つ。描く側（Sprite）はこれを見て描き方を選ぶ。
	 * 以前はテクスチャが知らず、描く側が毎回正しく指定する必要があった（間違えると縁が暗くなる・にじむ）。
	 */
	bool is_premultiplied() const { return premultiplied_; }
	void set_premultiplied( bool p ) { premultiplied_ = p; }

private:
	bool premultiplied_ = false;

public:
	virtual uint_t get_multi_sample_quality() const = 0;

}; // Texture

} // namespace core::graphics
