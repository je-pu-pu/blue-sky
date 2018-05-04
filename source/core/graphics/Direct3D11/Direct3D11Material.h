#pragma once

#include "Direct3D11.h"
#include <game/Material.h>

namespace game
{

class Shader;
class Texture;

}

class Direct3D11;

/**
 * Direct3D11Mesh ‚Ìƒ}ƒeƒŠƒAƒ‹
 *
 */
class Direct3D11Material : public game::Material
{
public:
	typedef game::Shader Shader;
	typedef game::Texture Texture;

protected:
	Direct3D11*			direct_3d_;

	const Shader*		shader_;
	const Texture*		texture_;

public:
	Direct3D11Material( Direct3D11* );
	virtual ~Direct3D11Material();

	const Shader* get_shader() const override { return shader_; }
	void set_shader( const Shader* s ) override { shader_ = s; }

	const Texture* get_texture() const override { return texture_; }
	void set_texture( const Texture* t ) override { texture_ = t; }

	void bind() const override;

}; // class Direct3D11Material
