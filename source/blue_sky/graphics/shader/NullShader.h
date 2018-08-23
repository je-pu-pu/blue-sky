#pragma once

#include "BaseShader.h"

namespace blue_sky::graphics::shader
{

/**
 * 何もしないシェーダー
 *
 */
class NullShader : public BaseShader
{
public:
	NullShader* clone() const override { return new NullShader( *this ); }

	Texture* get_texture_at( uint_t ) override { return nullptr; }
	const Texture* get_texture_at( uint_t ) const override { return nullptr; }
	void set_texture_at( uint_t, Texture* ) override { }

	void bind() const override { }
	void render_model( const game::Model* ) const override { }
	void render( const Mesh*, uint_t ) const override { }
};

} // namespace blue_sky::graphics::shader
