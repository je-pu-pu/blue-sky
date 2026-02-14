#include "EffectTechnique.h"
#include "EffectPass.h"
#include "Direct3D11.h"

#include <d3dx11effect.h>

namespace core::graphics::direct_3d_11
{

EffectTechnique::EffectTechnique( Direct3D* direct_3d, ID3DX11EffectTechnique* t )
{
	D3DX11_TECHNIQUE_DESC desc;

	DIRECT_X_FAIL_CHECK( t->GetDesc( & desc ) );

	for ( UINT n = 0; n < desc.Passes; n++ )
	{
		pass_list_.push_back( std::make_unique< EffectPass >( direct_3d, t->GetPassByIndex( n ) ) );
	}
}

EffectTechnique::~EffectTechnique() = default;

} // namespace core::graphics::direct_3d_11
