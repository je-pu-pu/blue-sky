#include "EffectTechnique.h"
#include "EffectPass.h"
#include "Direct3D11.h"

namespace core::graphics::direct_3d_11
{

EffectTechnique::EffectTechnique( Direct3D* direct_3d, const PassSetupList* setup_list )
{
	rebind( direct_3d, setup_list );
}

void EffectTechnique::rebind( Direct3D* direct_3d, const PassSetupList* setup_list )
{
	pass_list_.clear();

	if ( ! setup_list )
	{
		return;
	}

	for ( const auto& setup : *setup_list )
	{
		pass_list_.push_back( std::make_unique< EffectPass >( direct_3d, setup ) );
	}
}

EffectTechnique::~EffectTechnique() = default;

} // namespace core::graphics::direct_3d_11
