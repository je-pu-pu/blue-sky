#include "Direct3D11Effect.h"
#include "Direct3D11EffectTechnique.h"
#include "Direct3D11.h"
#include "DirectX.h"

#include "include/d3dx11effect.h"

#pragma comment( lib, "effects11.lib" )

Direct3D11Effect::Direct3D11Effect( Direct3D* direct_3d )
	: direct_3d_( direct_3d )
	, effect_( 0 )
{

}

Direct3D11Effect::~Direct3D11Effect()
{
	for ( TechniqueList::iterator i = technique_list_.begin(); i != technique_list_.end(); ++i )
	{
		delete i->second;
	}

	DIRECT_X_RELEASE( effect_ );
}

void Direct3D11Effect::load( const char* file_path )
{
	ID3D10Blob* shader = 0;
	ID3D10Blob* error_messages = 0;

	HRESULT hr = D3DX11CompileFromFile( file_path, 0, 0, 0, "fx_5_0", 0, 0, 0, & shader, & error_messages, 0  );

	if ( FAILED( hr ) )
	{
		if ( error_messages )
		{
			COMMON_THROW_EXCEPTION_MESSAGE( static_cast< char* >( error_messages->GetBufferPointer() ) );
		}
		else
		{
			DIRECT_X_FAIL_CHECK( hr );
		}
	}

	int n = shader->GetBufferSize();

	DIRECT_X_FAIL_CHECK( D3DX11CreateEffectFromMemory( shader->GetBufferPointer(), shader->GetBufferSize(), 0, direct_3d_->getDevice(), & effect_ ) );
}

Direct3D11Effect::Technique* Direct3D11Effect::getTechnique( const char* name )
{
	TechniqueList::iterator i = technique_list_.find( name );

	if ( i != technique_list_.end() )
	{
		return i->second;
	}

	ID3DX11EffectTechnique* t = effect_->GetTechniqueByName( name );

	if ( ! t->IsValid() )
	{
		return 0;
	}

	Technique* technique = new Technique( direct_3d_, t );

	technique_list_[ name ] = technique;

	return technique;
}