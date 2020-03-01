#pragma once

#include "ShaderResource.h"

namespace core::graphics
{

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
	virtual uint_t get_multi_sample_quality() const = 0;

}; // Texture

} // namespace core::graphics
