#pragma once

#include <type/type.h>

namespace game
{

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

	virtual void bind() const = 0;

}; // Material

} // namespace game
