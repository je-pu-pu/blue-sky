#include "Direct3D11Material.h"
#include "Direct3D11TextureManager.h"
#include "Direct3D11.h"

#include <blue_sky/graphics/shader/BypassShader.h>

#include <game/Shader.h>

#include <common/exception.h>
#include <common/math.h>

Direct3D11Material::Direct3D11Material()
	: shader_( std::make_unique< blue_sky::graphics::shader::BypassShader >() )
	, texture_( 0 )
{

}

Direct3D11Material::~Direct3D11Material()
{
	
}

#if 0
void Direct3D11Material::bind() const
{
	// shader_->bind();

	if ( texture_ )
	{
		texture_->bind_to_ps( 0 );
	}

	direct_3d_->getImmediateContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	// direct_3d_->getImmediateContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST );
}
#endif