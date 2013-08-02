#include "Direct3D11EffectTechnique.h"
#include "Direct3D11EffectPass.h"
#include "Direct3D11.h"
#include "DirectX.h"

#include "include/d3dx11effect.h"

#include "memory.h"

Direct3D11EffectTechnique::Direct3D11EffectTechnique( Direct3D* direct_3d, ID3DX11EffectTechnique* t )
{
	D3DX11_TECHNIQUE_DESC desc;

	DIRECT_X_FAIL_CHECK( t->GetDesc( & desc ) );

	for ( UINT n = 0; n < desc.Passes; n++ )
	{
		pass_list_.push_back( new Pass( direct_3d, t->GetPassByIndex( n ) ) );
	}
}

Direct3D11EffectTechnique::~Direct3D11EffectTechnique()
{
	for ( PassList::iterator i = pass_list_.begin(); i != pass_list_.end(); ++i )
	{
		delete *i;
	}
}
