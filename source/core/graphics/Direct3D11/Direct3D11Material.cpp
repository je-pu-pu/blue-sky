#include "Direct3D11Material.h"
#include "Direct3D11TextureManager.h"
#include "Direct3D11.h"

#include <game/Shader.h>

#include <common/exception.h>
#include <common/math.h>

Direct3D11Material::Direct3D11Material( Direct3D11* direct_3d )
	: direct_3d_( direct_3d )
	, shader_( 0 )
	, texture_( 0 )
{

}

Direct3D11Material::~Direct3D11Material()
{
	
}

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
