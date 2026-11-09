#include "Effect.h"
#include "EffectTechnique.h"
#include "Direct3D11.h"

#include <common/string.h>

#include <d3dx11effect.h>
#include <d3dcompiler.h>

#ifdef _DEBUG
#pragma comment( lib, "effects11d.lib" )
#else
#pragma comment( lib, "effects11.lib" )
#endif

namespace core::graphics::direct_3d_11
{

Effect::Effect( Direct3D* direct_3d )
	: direct_3d_( direct_3d )
	, effect_( 0 )
{

}

Effect::~Effect()
{
	clear();
}

void Effect::load( const char* file_path )
{
	ID3D10Blob* shader = nullptr;
	ID3D10Blob* error_messages = nullptr;

	const std::wstring ws_file_path = common::convert_to_wstring( file_path );

#ifdef _DEBUG
	const UINT flags1 = D3DCOMPILE_DEBUG;
#else
	const UINT flags1 = 0;
#endif

	const HRESULT hr = D3DCompileFromFile( ws_file_path.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "", "fx_5_0", flags1, 0, & shader, & error_messages );

	if ( FAILED( hr ) )
	{
		// 失敗しても、それまでのエフェクトはそのまま残る
		std::string message = error_messages ? static_cast< const char* >( error_messages->GetBufferPointer() ) : std::string();

		DIRECT_X_RELEASE( shader );
		DIRECT_X_RELEASE( error_messages );

		if ( ! message.empty() )
		{
			COMMON_THROW_EXCEPTION_MESSAGE( message );
		}

		DIRECT_X_FAIL_CHECK( hr );
	}

	DIRECT_X_RELEASE( error_messages );		// 警告だけのときも返ってくるので解放する（以前はリークしていた）

	ID3DX11Effect* new_effect = nullptr;
	const HRESULT create_hr = D3DX11CreateEffectFromMemory( shader->GetBufferPointer(), shader->GetBufferSize(), 0, direct_3d_->getDevice(), & new_effect );

	DIRECT_X_RELEASE( shader );
	DIRECT_X_FAIL_CHECK( create_hr );

	// 成功したので差し替える。取得済みの technique は、オブジェクトを残したまま中身だけ入れ替える
	ID3DX11Effect* old_effect = effect_;
	effect_ = new_effect;

	for ( auto& [ name, technique ] : technique_list_ )
	{
		technique->rebind( direct_3d_, effect_->GetTechniqueByName( name.c_str() ) );
	}

	DIRECT_X_RELEASE( old_effect );
}

/**
 * 全てのリソースをクリアする
 * 
 */
void Effect::clear()
{
	technique_list_.clear();

	DIRECT_X_RELEASE( effect_ );
}

Effect::Technique* Effect::get_technique( const char* name )
{
	if ( ! effect_ || ! name )
	{
		return nullptr;
	}

	TechniqueList::iterator i = technique_list_.find( name );

	if ( i != technique_list_.end() )
	{
		return i->second.get();
	}

	ID3DX11EffectTechnique* t = effect_->GetTechniqueByName( name );

	if ( ! t->IsValid() )
	{
		return nullptr;
	}

	auto technique = std::make_unique< Technique >( direct_3d_, t );
	Technique* result = technique.get();

	technique_list_[ name ] = std::move( technique );

	return result;
}

} // namespace core::graphics::direct_3d_11
