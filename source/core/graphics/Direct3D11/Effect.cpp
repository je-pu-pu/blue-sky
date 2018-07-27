#include "Effect.h"
#include "EffectTechnique.h"
#include "Direct3D11.h"

#include "d3dx11effect.h"

#include <common/string.h>

#include <d3dcompiler.h>

#pragma comment( lib, "effects11.lib" )

namespace core::graphics::direct_3d_11
{

Effect::Effect( Direct3D* direct_3d )
	: direct_3d_( direct_3d )
	, effect_( 0 )
{

}

Effect::~Effect()
{
	for ( TechniqueList::iterator i = technique_list_.begin(); i != technique_list_.end(); ++i )
	{
		delete i->second;
	}

	DIRECT_X_RELEASE( effect_ );
}

void Effect::load( const char* file_path )
{
	ID3D10Blob* shader = 0;
	ID3D10Blob* error_messages = 0;

	std::wstring ws_file_path = common::convert_to_wstring( file_path );

	HRESULT hr = D3DCompileFromFile( ws_file_path.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, nullptr, "fx_5_0", 0, 0, & shader, & error_messages );

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

	DIRECT_X_FAIL_CHECK( D3DX11CreateEffectFromMemory( shader->GetBufferPointer(), shader->GetBufferSize(), 0, direct_3d_->getDevice(), & effect_ ) );
	
	/*
	int n = shader->GetBufferSize();

	for ( int n = 0; ; ++n )
	{
		D3DX11_EFFECT_VARIABLE_DESC evd;
		DIRECT_X_FAIL_CHECK( effect_->GetVariableByIndex( n )->GetDesc( & evd ) );

		const char* name = evd.Name;
	}

	bool a = effect_->IsValid();
	bool b = effect_->GetVariableByName( "|vs_skin" )->IsValid();
	bool c = effect_->GetVariableByName( "|vs_skin" )->AsShader()->IsValid();

	ID3D11VertexShader* vs = 0;
	effect_->GetVariableByName( "|vs_skin" )->AsShader()->GetVertexShader( 0, & vs );

	vs = vs;
	*/
}

Effect::Technique* Effect::get_technique( const char* name )
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

} // namespace core::graphics::direct_3d_11
