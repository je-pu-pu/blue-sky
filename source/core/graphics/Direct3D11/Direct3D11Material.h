#pragma once

#include "Direct3D11.h"

#include <game/Material.h>
#include <blue_sky/graphics/shader/BaseShader.h>

namespace game
{

class Shader;
class Texture;

}

class Direct3D11;

/**
 * Direct3D11Mesh のマテリアル
 *
 * @todo マテリアルの存在意義を考える ( Mesh が直接 Shader を持てばいいのでは？ )
 */
class Direct3D11Material : public game::Material
{
public:
	typedef blue_sky::graphics::shader::BaseShader Shader;
	typedef game::Texture Texture;

protected:
	std::unique_ptr< const Shader > shader_;
	const Texture* texture_;

public:
	Direct3D11Material();
	virtual ~Direct3D11Material();

	const Shader* get_shader() const override { return shader_.get(); }
	void set_shader( const game::Shader* s ) override { shader_.reset( static_cast< const Shader* >( s ) ); }

	const Texture* get_texture() const override { return texture_; }
	void set_texture( const Texture* t ) override { texture_ = t; }

	// void bind() const override;

}; // class Direct3D11Material
