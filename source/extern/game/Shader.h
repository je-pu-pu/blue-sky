#pragma once

#include "Texture.h"
#include <type/type.h>

namespace game
{

class Mesh;
class Texture;

/**
 * 抽象シェーダークラス
 *
 */
class Shader
{
public:
	Shader() { }
	virtual ~Shader() { }

	virtual Shader* clone() const = 0;

	virtual Texture* get_texture_at( uint_t ) = 0;
	virtual void set_texture_at( uint_t, Texture* ) = 0;

	virtual void bind() const = 0;
	virtual void render( const Mesh*, uint_t ) const = 0;

}; // Material

} // namespace game
