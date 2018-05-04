#ifndef GAME_MATERIAL_H
#define GAME_MATERIAL_H

#include <type/type.h>

namespace game
{

class Shader;
class Texture;

/**
 * ’ŠÛ¿Š´ƒNƒ‰ƒX
 *
 */
class Material
{
public:
	Material() { }
	virtual ~Material() { }

	virtual const Shader* get_shader() const = 0;
	virtual void set_shader( const Shader* ) = 0;

	virtual const Texture* get_texture() const = 0;
	virtual void set_texture( const Texture* ) = 0;

	virtual void bind() const = 0;

}; // Material

} // namespace game

#endif // GAME_MATERIAL_H
