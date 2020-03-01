#pragma once

#include <type/type.h>

namespace core::graphics
{
	class Model;
	class Mesh;
	class ShaderResource;

/**
 * 抽象シェーダークラス
 *
 */
class Shader
{
public:
	using Texture = ShaderResource;

public:
	Shader() { }
	virtual ~Shader() { }

	virtual Shader* clone() const = 0;

	virtual Texture* get_texture_at( uint_t ) = 0;
	virtual const Texture* get_texture_at( uint_t ) const = 0;
	virtual void set_texture_at( uint_t, Texture* ) = 0;

	virtual void bind() const = 0;
	virtual void render_model( const Model* ) const = 0;
	virtual void render( const Mesh*, uint_t ) const = 0;

}; // Shader

} // namespace core::graphics
