#include "Effect.h"
#include "EffectTechnique.h"
#include "EffectStateDesc.h"
#include "Direct3D11.h"

#include <d3dcompiler.h>
#include <d3d11shader.h>

#include <algorithm>
#include <atomic>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <thread>

#pragma comment( lib, "d3dcompiler.lib" )

namespace core::graphics::direct_3d_11
{

namespace
{

template< typename T > using ComPtr = Microsoft::WRL::ComPtr< T >;

bool read_file( const std::filesystem::path& path, std::string& out )
{
	std::ifstream in( path, std::ios::binary );

	if ( ! in )
	{
		return false;
	}

	std::ostringstream ss;
	ss << in.rdbuf();
	out = ss.str();

	return true;
}

/**
 * #include を、書いたファイルのあるディレクトリから探す
 *
 * D3DPreprocess に渡す。読んだ中身は、プリプロセスが終わるまでこのオブジェクトが持つ。
 */
class IncludeHandler : public ID3DInclude
{
private:
	std::filesystem::path root_directory_;
	std::map< const void*, std::filesystem::path > directory_of_data_;
	std::vector< std::unique_ptr< std::string > > buffer_list_;

public:
	explicit IncludeHandler( const std::filesystem::path& root_directory )
		: root_directory_( root_directory )
	{ }

	HRESULT __stdcall Open( D3D_INCLUDE_TYPE, LPCSTR file_name, LPCVOID parent_data, LPCVOID* data, UINT* bytes ) override
	{
		const auto i = directory_of_data_.find( parent_data );
		const std::filesystem::path directory = i != directory_of_data_.end() ? i->second : root_directory_;

		std::filesystem::path path = directory / file_name;
		auto buffer = std::make_unique< std::string >();

		if ( ! read_file( path, *buffer ) )
		{
			path = root_directory_ / file_name;

			if ( ! read_file( path, *buffer ) )
			{
				return E_FAIL;
			}
		}

		*data = buffer->data();
		*bytes = static_cast< UINT >( buffer->size() );

		directory_of_data_[ buffer->data() ] = path.parent_path();
		buffer_list_.push_back( std::move( buffer ) );

		return S_OK;
	}

	HRESULT __stdcall Close( LPCVOID ) override
	{
		return S_OK;	// 中身はプリプロセスの終わりにまとめて解放する
	}
};

std::string blob_to_string( ID3DBlob* blob )
{
	if ( ! blob || blob->GetBufferSize() == 0 )
	{
		return std::string();
	}

	std::string s( static_cast< const char* >( blob->GetBufferPointer() ), blob->GetBufferSize() );

	while ( ! s.empty() && ( s.back() == '\0' || s.back() == '\n' || s.back() == '\r' ) )
	{
		s.pop_back();
	}

	return s;
}

std::string join( const std::vector< std::string >& list )
{
	std::string s;

	for ( const auto& line : list )
	{
		s += line + "\n";
	}

	return s;
}

/// シェーダーを作り、使うサンプラーのスロットをリフレクションで調べる
struct ShaderBuilder
{
	ID3D11Device* device;
	const std::map< std::string, D3D11_SAMPLER_DESC >& sampler_desc_list;
	std::map< std::string, ComPtr< ID3D11SamplerState > > sampler_list;
	std::vector< std::string >& warnings;

	ComPtr< ID3D11SamplerState > get_sampler( const std::string& name )
	{
		if ( auto i = sampler_list.find( name ); i != sampler_list.end() )
		{
			return i->second;
		}

		ComPtr< ID3D11SamplerState > sampler;
		const auto i = sampler_desc_list.find( name );

		if ( i == sampler_desc_list.end() )
		{
			warnings.push_back( "サンプラー " + name + " の状態ブロックがありません（結び付けません）" );
		}
		else
		{
			DIRECT_X_FAIL_CHECK( device->CreateSamplerState( & i->second, & sampler ) );
		}

		sampler_list[ name ] = sampler;

		return sampler;
	}

	std::shared_ptr< EffectShader > build( effect::ShaderStage stage, const effect::ShaderRef& ref, ID3DBlob* code )
	{
		auto shader = std::make_shared< EffectShader >();
		shader->stage = stage;
		shader->byte_code = code;

		const void* p = code->GetBufferPointer();
		const SIZE_T n = code->GetBufferSize();

		switch ( stage )
		{
			case effect::ShaderStage::VS: { ComPtr< ID3D11VertexShader > s; DIRECT_X_FAIL_CHECK( device->CreateVertexShader( p, n, nullptr, & s ) ); shader->object = s; break; }
			case effect::ShaderStage::HS: { ComPtr< ID3D11HullShader > s; DIRECT_X_FAIL_CHECK( device->CreateHullShader( p, n, nullptr, & s ) ); shader->object = s; break; }
			case effect::ShaderStage::DS: { ComPtr< ID3D11DomainShader > s; DIRECT_X_FAIL_CHECK( device->CreateDomainShader( p, n, nullptr, & s ) ); shader->object = s; break; }
			case effect::ShaderStage::GS: { ComPtr< ID3D11GeometryShader > s; DIRECT_X_FAIL_CHECK( device->CreateGeometryShader( p, n, nullptr, & s ) ); shader->object = s; break; }
			case effect::ShaderStage::PS: { ComPtr< ID3D11PixelShader > s; DIRECT_X_FAIL_CHECK( device->CreatePixelShader( p, n, nullptr, & s ) ); shader->object = s; break; }
		}

		ComPtr< ID3D11ShaderReflection > reflection;
		DIRECT_X_FAIL_CHECK( D3DReflect( p, n, __uuidof( ID3D11ShaderReflection ), reinterpret_cast< void** >( reflection.GetAddressOf() ) ) );

		D3D11_SHADER_DESC desc { };
		DIRECT_X_FAIL_CHECK( reflection->GetDesc( & desc ) );

		for ( UINT i = 0; i < desc.BoundResources; i++ )
		{
			D3D11_SHADER_INPUT_BIND_DESC bind { };
			DIRECT_X_FAIL_CHECK( reflection->GetResourceBindingDesc( i, & bind ) );

			if ( bind.Type == D3D_SIT_SAMPLER )
			{
				if ( bind.BindCount != 1 )
				{
					warnings.push_back( ref.key() + ": サンプラーの配列（" + bind.Name + "）には対応していません（先頭だけ結び付けます）" );
				}

				if ( auto sampler = get_sampler( bind.Name ) )
				{
					shader->sampler_list.emplace_back( bind.BindPoint, sampler );
				}
			}
			else if ( bind.Type == D3D_SIT_CBUFFER && std::strcmp( bind.Name, "$Globals" ) == 0 )
			{
				// effect は初期値を入れた $Globals を結び付けていたが、今は何も結び付けない
				warnings.push_back( ref.key() + ": cbuffer に入っていないグローバル変数を使っています（$Globals は結び付けません。static const にするか cbuffer に入れてください）" );
			}
		}

		return shader;
	}
};

} // namespace

Effect::Effect( Direct3D* direct_3d )
	: direct_3d_( direct_3d )
{

}

Effect::~Effect()
{
	clear();
}

std::string Effect::preprocess( const char* file_path )
{
	const std::filesystem::path path( file_path );
	std::string source;

	if ( ! read_file( path, source ) )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( std::string( "エフェクトファイルを開けません : " ) + file_path );
	}

	IncludeHandler include( path.parent_path() );
	ComPtr< ID3DBlob > text;
	ComPtr< ID3DBlob > errors;

	const HRESULT hr = D3DPreprocess( source.data(), source.size(), file_path, nullptr, & include, & text, & errors );

	if ( FAILED( hr ) )
	{
		const std::string message = blob_to_string( errors.Get() );

		if ( ! message.empty() )
		{
			COMMON_THROW_EXCEPTION_MESSAGE( message );
		}

		DIRECT_X_FAIL_CHECK( hr );
	}

	return blob_to_string( text.Get() );
}

ComPtr< ID3DBlob > Effect::compile_shader( const std::string& source, const char* source_name, const effect::ShaderRef& ref, std::string& message )
{
#ifdef _DEBUG
	const UINT flags = D3DCOMPILE_DEBUG;
#else
	const UINT flags = 0;
#endif

	ComPtr< ID3DBlob > code;
	ComPtr< ID3DBlob > errors;

	const HRESULT hr = D3DCompile( source.data(), source.size(), source_name, nullptr, nullptr, ref.entry_point.c_str(), ref.profile.c_str(), flags, 0, & code, & errors );

	message = blob_to_string( errors.Get() );

	if ( FAILED( hr ) )
	{
		if ( message.empty() )
		{
			message = "HRESULT " + std::to_string( static_cast< unsigned long >( hr ) );
		}

		return nullptr;
	}

	return code;
}

void Effect::load( const char* file_path )
{
	// 1. 読む
	const std::string source = preprocess( file_path );
	const effect::ParseResult parsed = effect::parse( source );

	if ( ! parsed.errors.empty() )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( std::string( "エフェクトファイルを読めません : " ) + file_path + "\n" + join( parsed.errors ) );
	}

	std::vector< std::string > warnings = parsed.warnings;

	// 2. 状態のブロックを desc にする（知らないキーをすべて知らせるため、使われていないものも変換する）
	std::map< std::string, D3D11_BLEND_DESC > blend_desc_list;
	std::map< std::string, D3D11_DEPTH_STENCIL_DESC > depth_stencil_desc_list;
	std::map< std::string, D3D11_RASTERIZER_DESC > rasterizer_desc_list;
	std::map< std::string, D3D11_SAMPLER_DESC > sampler_desc_list;

	for ( const auto& block : parsed.state_blocks )
	{
		switch ( block.kind )
		{
			case effect::StateKind::BLEND: blend_desc_list[ block.name ] = make_blend_desc( block, warnings ); break;
			case effect::StateKind::DEPTH_STENCIL: depth_stencil_desc_list[ block.name ] = make_depth_stencil_desc( block, warnings ); break;
			case effect::StateKind::RASTERIZER: rasterizer_desc_list[ block.name ] = make_rasterizer_desc( block, warnings ); break;
			case effect::StateKind::SAMPLER: sampler_desc_list[ block.name ] = make_sampler_desc( block, warnings ); break;
		}
	}

	// 3. シェーダーを重複なく集めて、並列にコンパイルする（数が多く、1 つずつだと起動と読み直しが遅い）
	struct ShaderItem
	{
		effect::ShaderStage stage;
		effect::ShaderRef ref;
		ComPtr< ID3DBlob > code;
		std::string message;
	};

	std::map< std::string, ShaderItem > shader_item_list;

	for ( const auto& technique : parsed.techniques )
	{
		for ( const auto& pass : technique.passes )
		{
			for ( size_t n = 0; n < pass.shaders.size(); n++ )
			{
				if ( pass.shaders[ n ] && ! pass.shaders[ n ]->is_null() )
				{
					shader_item_list.try_emplace( pass.shaders[ n ]->key(), ShaderItem { static_cast< effect::ShaderStage >( n ), *pass.shaders[ n ] } );
				}
			}
		}
	}

	{
		std::vector< ShaderItem* > items;

		for ( auto& [ key, item ] : shader_item_list )
		{
			items.push_back( & item );
		}

		std::atomic< size_t > next { 0 };
		const size_t thread_count = std::max< size_t >( 1, std::min< size_t >( std::thread::hardware_concurrency(), items.size() ) );
		std::vector< std::thread > threads;

		for ( size_t t = 0; t < thread_count; t++ )
		{
			threads.emplace_back( [&] () {
				for ( size_t i = next++; i < items.size(); i = next++ )
				{
					items[ i ]->code = compile_shader( parsed.shader_source, file_path, items[ i ]->ref, items[ i ]->message );
				}
			} );
		}

		for ( auto& t : threads )
		{
			t.join();
		}
	}

	// 同じエラー・警告が複数のシェーダーから出る（共通の .hlsl の誤りなど）のでまとめる
	std::map< std::string, std::vector< std::string > > compile_error_list;	// メッセージ → シェーダー
	std::set< std::string > compile_warnings;

	for ( const auto& [ key, item ] : shader_item_list )
	{
		if ( ! item.code )
		{
			compile_error_list[ item.message ].push_back( key );
		}
		else if ( ! item.message.empty() )
		{
			compile_warnings.insert( item.message );
		}
	}

	if ( ! compile_error_list.empty() )
	{
		std::vector< std::string > compile_errors;

		for ( const auto& [ message, keys ] : compile_error_list )
		{
			std::string targets;

			for ( const auto& key : keys )
			{
				targets += ( targets.empty() ? "" : ", " ) + key;
			}

			compile_errors.push_back( message + "\n（対象 : " + targets + "）" );
		}

		COMMON_THROW_EXCEPTION_MESSAGE( join( compile_errors ) );
	}

	for ( const auto& w : compile_warnings )
	{
		warnings.push_back( "コンパイルの警告 : " + w );
	}

	// 4. シェーダーと状態のオブジェクトを作る
	ID3D11Device* device = direct_3d_->getDevice();

	ShaderBuilder builder { device, sampler_desc_list, { }, warnings };
	std::map< std::string, std::shared_ptr< const EffectShader > > shader_list;

	for ( const auto& [ key, item ] : shader_item_list )
	{
		shader_list[ key ] = builder.build( item.stage, item.ref, item.code.Get() );
	}

	std::vector< std::string > errors;

	std::map< std::string, ComPtr< ID3D11BlendState > > blend_state_list;
	std::map< std::string, ComPtr< ID3D11DepthStencilState > > depth_stencil_state_list;
	std::map< std::string, ComPtr< ID3D11RasterizerState > > rasterizer_state_list;

	// 名前から状態オブジェクトを作る（同じ名前は共有する）。名前が空なら NULL（D3D の既定）
	auto get_state = [&] ( auto& state_list, const auto& desc_list, const std::string& name, const char* kind, auto create ) {
		using State = typename std::remove_reference_t< decltype( state_list ) >::mapped_type;

		if ( name.empty() )
		{
			return State();
		}

		if ( auto i = state_list.find( name ); i != state_list.end() )
		{
			return i->second;
		}

		const auto d = desc_list.find( name );

		if ( d == desc_list.end() )
		{
			errors.push_back( std::string( kind ) + " " + name + " がありません" );
			return State();
		}

		State state;
		DIRECT_X_FAIL_CHECK( create( d->second, state ) );
		state_list[ name ] = state;

		return state;
	};

	TechniqueSetupList setup_list;

	for ( const auto& technique : parsed.techniques )
	{
		auto& pass_setup_list = setup_list[ technique.name ];

		for ( const auto& pass : technique.passes )
		{
			EffectPassSetup setup;

			for ( size_t n = 0; n < pass.shaders.size(); n++ )
			{
				if ( pass.shaders[ n ] )
				{
					setup.shader_list[ n ] = pass.shaders[ n ]->is_null() ? nullptr : shader_list.at( pass.shaders[ n ]->key() );
				}
			}

			if ( pass.blend_state.is_set )
			{
				setup.has_blend_state = true;
				setup.blend_state = get_state( blend_state_list, blend_desc_list, pass.blend_state.name, "BlendState",
					[&] ( const D3D11_BLEND_DESC& d, ComPtr< ID3D11BlendState >& s ) { return device->CreateBlendState( & d, & s ); } );
				setup.blend_factor = pass.blend_factor;
				setup.sample_mask = pass.sample_mask;
			}

			if ( pass.depth_stencil_state.is_set )
			{
				setup.has_depth_stencil_state = true;
				setup.depth_stencil_state = get_state( depth_stencil_state_list, depth_stencil_desc_list, pass.depth_stencil_state.name, "DepthStencilState",
					[&] ( const D3D11_DEPTH_STENCIL_DESC& d, ComPtr< ID3D11DepthStencilState >& s ) { return device->CreateDepthStencilState( & d, & s ); } );
				setup.stencil_ref = pass.stencil_ref;
			}

			if ( pass.rasterizer_state.is_set )
			{
				setup.has_rasterizer_state = true;
				setup.rasterizer_state = get_state( rasterizer_state_list, rasterizer_desc_list, pass.rasterizer_state.name, "RasterizerState",
					[&] ( const D3D11_RASTERIZER_DESC& d, ComPtr< ID3D11RasterizerState >& s ) { return device->CreateRasterizerState( & d, & s ); } );
			}

			pass_setup_list.push_back( std::move( setup ) );
		}
	}

	if ( ! errors.empty() )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( std::string( "エフェクトファイルを読めません : " ) + file_path + "\n" + join( errors ) );
	}

	// 複数行の警告（コンパイラのもの）も、ログで探せるように 1 行ずつ印を付ける
	for ( const auto& w : warnings )
	{
		std::istringstream lines( w );
		std::string line;

		while ( std::getline( lines, line ) )
		{
			if ( ! line.empty() && line.back() == '\r' )
			{
				line.pop_back();
			}

			std::cout << "[Effect] 警告 : " << line << std::endl;
		}
	}

	// 5. 成功したので差し替える。取得済みの technique は、オブジェクトを残したまま中身だけ入れ替える
	technique_setup_list_ = std::move( setup_list );
	loaded_ = true;

	for ( auto& [ name, technique ] : technique_list_ )
	{
		const auto i = technique_setup_list_.find( name );
		technique->rebind( direct_3d_, i != technique_setup_list_.end() ? & i->second : nullptr );
	}

	std::cout << "[Effect] " << file_path << " を読みました（technique " << parsed.techniques.size() << " 個・シェーダー " << shader_list.size() << " 個）" << std::endl;
}

/**
 * 全てのリソースをクリアする
 *
 */
void Effect::clear()
{
	technique_list_.clear();
	technique_setup_list_.clear();
	loaded_ = false;
}

Effect::Technique* Effect::get_technique( const char* name )
{
	if ( ! loaded_ || ! name )
	{
		return nullptr;
	}

	// FX11 のグループ付きの名前（"グループ|technique"）。グループは使っていないので | の後ろだけを見る
	std::string technique_name = name;

	if ( const size_t bar = technique_name.rfind( '|' ); bar != std::string::npos )
	{
		technique_name = technique_name.substr( bar + 1 );
	}

	TechniqueList::iterator i = technique_list_.find( technique_name );

	if ( i != technique_list_.end() )
	{
		return i->second.get();
	}

	const auto setup = technique_setup_list_.find( technique_name );

	if ( setup == technique_setup_list_.end() )
	{
		return nullptr;
	}

	auto technique = std::make_unique< Technique >( direct_3d_, & setup->second );
	Technique* result = technique.get();

	technique_list_[ technique_name ] = std::move( technique );

	return result;
}

} // namespace core::graphics::direct_3d_11
