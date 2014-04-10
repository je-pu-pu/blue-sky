#ifndef GAME_MATERIAL_H
#define GAME_MATERIAL_H

#include <type/type.h>

namespace game
{

class Texture;

/**
 * ���ێ����N���X
 *
 */
class Material
{
public:
	Material() { }
	virtual ~Material() { }

	virtual const Texture* get_texture() const = 0;
	virtual void set_texture( const Texture* ) = 0;
	
	virtual void bind_to_ia() const = 0;
	virtual void render() const = 0;

}; // Material

} // namespace game

#endif // GAME_MATERIAL_H
