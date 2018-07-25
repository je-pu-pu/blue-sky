#pragma once

#include <blue_sky/graphics/shader/BaseShader.h>

namespace blue_sky::graphics::shader
{

/**
 * �V�F�[�_�[�p�����[�^�[�̐ݒ���s�킸�A�P�� Mesh::render( uint_t ) ���Ăяo���V�F�[�_�[
 * ( ���̃V�F�[�_�[�͌Â��R�[�h�Ƃ̌݊����̂��߂����Ɏg���� )
 */
class BypassShader : public BaseShader
{
public:
	BypassShader* clone() const override { return new BypassShader( *this ); }

	Texture* get_texture_at( uint_t ) override { return nullptr; }
	const Texture* get_texture_at( uint_t ) const override { return nullptr; }
	void set_texture_at( uint_t, Texture* ) override { }

	void bind() const override { }

	void render( const Mesh* mesh, uint_t n ) const override
	{
		get_graphics_manager()->set_primitive_topology( PrimitiveTopology::TRIANGLE_LIST );
		mesh->render( n );
	}
};

} // namespace blue_sky::graphics::shader
