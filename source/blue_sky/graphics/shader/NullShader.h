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
	using BaseShader::BaseShader;

	void reload() override { }

	NullShader* clone() const override { return new NullShader( *this ); }

	Texture* get_texture_at( uint_t ) override { return nullptr; }
	const Texture* get_texture_at( uint_t ) const override { return nullptr; }
	void set_texture_at( uint_t, Texture* ) override { }

	void bind() const override { }
	void render_model( const core::graphics::Model* ) const override { }
	void render( const Mesh*, uint_t ) const override { }

	/// 何も描かないので、まとめて描くときも何もしない（まとめて描くモデルの、使わない頂点のまとまりに入っていることがある）
	bool can_render_object_instances() const override { return true; }
	void render_object_instances( const Mesh*, uint_t, uint_t ) const override { }
};

} // namespace blue_sky::graphics::shader
