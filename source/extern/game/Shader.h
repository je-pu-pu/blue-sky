#pragma once

#include <type/type.h>

namespace game
{

class Texture;

/**
 * ���ۃV�F�[�_�[�N���X
 *
 */
class Shader
{
public:
	Shader() { }
	virtual ~Shader() { }

	virtual void bind() const = 0;

}; // Material

} // namespace game
