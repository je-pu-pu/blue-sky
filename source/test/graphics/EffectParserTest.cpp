#include "pch.h"
#include <core/graphics/EffectParser.h>
#include <core/graphics/Direct3D11/EffectStateDesc.h>
#include <core/graphics/Direct3D11/Effect.h>

#include <algorithm>
#include <filesystem>
#include <set>

using namespace core::graphics;
using namespace core::graphics::effect;

namespace {

size_t stage_index( ShaderStage s ) { return static_cast< size_t >( s ); }

size_t count_lines( const std::string& s ) { return std::count( s.begin(), s.end(), '\n' ); }

bool contains( const std::vector< std::string >& list, const std::string& part )
{
	return std::any_of( list.begin(), list.end(), [&] ( const std::string& s ) { return s.find( part ) != std::string::npos; } );
}

const char* const SIMPLE_EFFECT = R"(
cbuffer A : register( b0 ) { float4 Color; };
Texture2D tex : register( t0 );

SamplerState linear_sampler
{
	Filter = MIN_MAG_MIP_LINEAR; // コメント
	AddressU = Wrap;
};

BlendState Blend
{
	BlendEnable[ 0 ] = True;
	SrcBlend = SRC_ALPHA;
	DestBlend = INV_SRC_ALPHA;
};

float4 vs_main( float4 p : SV_Position ) : SV_Position { return p; }
float4 ps_main( float4 p : SV_Position ) : SV_Target { return tex.Sample( linear_sampler, p.xy ) * Color; }

/*
technique11 commented_out { pass main { } }
*/

technique11 main
{
	pass first
	{
		SetBlendState( Blend, float4( 0.0f, 0.5f, 0.0f, 1.0f ), 0xFFFFFFFF );
		SetDepthStencilState( NoDepth, 3 );

		SetVertexShader( CompileShader( vs_4_0, vs_main() ) );
		SetHullShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, ps_main() ) );

		RASTERIZERSTATE = Default;
	}

	pass second
	{
		SetRasterizerState( NULL );
	}
}
)";

} // namespace

TEST( EffectParserTest, ReadsTechniquePassAndSetters )
{
	const auto r = parse( SIMPLE_EFFECT );

	EXPECT_TRUE( r.errors.empty() );
	EXPECT_TRUE( r.warnings.empty() );
	ASSERT_EQ( r.techniques.size(), 1u );	// コメントの中の technique は読まない

	const auto* t = r.find_technique( "main" );
	ASSERT_NE( t, nullptr );
	ASSERT_EQ( t->passes.size(), 2u );

	const auto& p = t->passes[ 0 ];
	EXPECT_EQ( p.name, "first" );

	EXPECT_TRUE( p.blend_state.is_set );
	EXPECT_EQ( p.blend_state.name, "Blend" );
	EXPECT_FLOAT_EQ( p.blend_factor[ 1 ], 0.5f );
	EXPECT_FLOAT_EQ( p.blend_factor[ 3 ], 1.0f );
	EXPECT_EQ( p.sample_mask, 0xFFFFFFFFu );

	EXPECT_TRUE( p.depth_stencil_state.is_set );
	EXPECT_EQ( p.depth_stencil_state.name, "NoDepth" );
	EXPECT_EQ( p.stencil_ref, 3u );

	// RASTERIZERSTATE = 名前; の書き方（大文字小文字を区別しない）
	EXPECT_TRUE( p.rasterizer_state.is_set );
	EXPECT_EQ( p.rasterizer_state.name, "Default" );

	ASSERT_TRUE( p.shaders[ stage_index( ShaderStage::VS ) ].has_value() );
	EXPECT_EQ( p.shaders[ stage_index( ShaderStage::VS ) ]->profile, "vs_4_0" );
	EXPECT_EQ( p.shaders[ stage_index( ShaderStage::VS ) ]->function, "vs_main" );
	EXPECT_EQ( p.shaders[ stage_index( ShaderStage::VS ) ]->entry_point, "vs_main" );

	// NULL は「外す」、書いていないステージは「触らない」
	ASSERT_TRUE( p.shaders[ stage_index( ShaderStage::HS ) ].has_value() );
	EXPECT_TRUE( p.shaders[ stage_index( ShaderStage::HS ) ]->is_null() );
	EXPECT_FALSE( p.shaders[ stage_index( ShaderStage::DS ) ].has_value() );
	EXPECT_FALSE( p.shaders[ stage_index( ShaderStage::GS ) ].has_value() );

	// 2 つ目のパス : NULL の状態は「D3D の既定に戻す」、書いていない状態は「触らない」
	const auto& p2 = t->passes[ 1 ];
	EXPECT_TRUE( p2.rasterizer_state.is_set );
	EXPECT_TRUE( p2.rasterizer_state.name.empty() );
	EXPECT_FALSE( p2.blend_state.is_set );
	EXPECT_FALSE( p2.shaders[ stage_index( ShaderStage::VS ) ].has_value() );
}

TEST( EffectParserTest, ReadsStateBlocks )
{
	const auto r = parse( SIMPLE_EFFECT );

	const auto* blend = r.find_state_block( StateKind::BLEND, "Blend" );
	ASSERT_NE( blend, nullptr );
	ASSERT_EQ( blend->assignments.size(), 3u );
	EXPECT_EQ( blend->assignments[ 0 ].key, "BlendEnable" );
	EXPECT_EQ( blend->assignments[ 0 ].index, 0 );
	EXPECT_EQ( blend->assignments[ 0 ].value, "True" );
	EXPECT_EQ( blend->assignments[ 1 ].index, -1 );
	EXPECT_EQ( blend->assignments[ 1 ].value, "SRC_ALPHA" );

	const auto* sampler = r.find_state_block( StateKind::SAMPLER, "linear_sampler" );
	ASSERT_NE( sampler, nullptr );
	EXPECT_EQ( sampler->assignments.size(), 2u );	// コメントは読まない
}

TEST( EffectParserTest, ShaderSourceKeepsSamplerDeclarationAndLines )
{
	const std::string source = SIMPLE_EFFECT;
	const auto r = parse( source );

	// 行の位置は変えない（コンパイルエラーの行番号が元のファイルと合うように）
	EXPECT_EQ( count_lines( r.shader_source ), count_lines( source ) );

	// technique と BlendState は消え、SamplerState は宣言だけが残る
	EXPECT_EQ( r.shader_source.find( "technique11 main" ), std::string::npos );
	EXPECT_EQ( r.shader_source.find( "BlendState" ), std::string::npos );
	EXPECT_EQ( r.shader_source.find( "Filter" ), std::string::npos );
	EXPECT_NE( r.shader_source.find( "SamplerState linear_sampler" ), std::string::npos );
	EXPECT_NE( r.shader_source.find( "float4 ps_main" ), std::string::npos );
}

TEST( EffectParserTest, ReportsUnsupportedPassStatement )
{
	const auto r = parse( "technique11 t { pass p { SetComputeShader( NULL ); } }" );

	EXPECT_TRUE( r.errors.empty() );
	EXPECT_TRUE( contains( r.warnings, "SetComputeShader" ) );
}

TEST( EffectParserTest, ReportsUnsupportedShaderExpression )
{
	const auto r = parse( "technique11 t { pass p { SetVertexShader( my_shader_variable ); } }" );

	EXPECT_TRUE( contains( r.errors, "my_shader_variable" ) );
}

TEST( EffectParserTest, GeneratesWrapperForEntryWithArguments )
{
	const std::string source = R"(
float4 ps_tint( uniform bool bright, float4 p : SV_Position, uniform float scale ) : SV_Target
{
	return bright ? p * scale : p;
}

technique11 t
{
	pass a { SetPixelShader( CompileShader( ps_4_0, ps_tint( true, 2.0f ) ) ); }
	pass b { SetPixelShader( CompileShader( ps_4_0, ps_tint( true, 2.0f ) ) ); }
	pass c { SetPixelShader( CompileShader( ps_4_0, ps_tint( false, 1.0f ) ) ); }
}
)";

	const auto r = parse( source );

	ASSERT_TRUE( r.errors.empty() );

	const auto& passes = r.find_technique( "t" )->passes;
	const auto& a = *passes[ 0 ].shaders[ stage_index( ShaderStage::PS ) ];
	const auto& b = *passes[ 1 ].shaders[ stage_index( ShaderStage::PS ) ];
	const auto& c = *passes[ 2 ].shaders[ stage_index( ShaderStage::PS ) ];

	ASSERT_EQ( a.arguments.size(), 2u );
	EXPECT_EQ( a.arguments[ 0 ], "true" );
	EXPECT_EQ( a.arguments[ 1 ], "2.0f" );

	// 同じ引数なら同じラッパーを共有する。違う引数なら別のラッパー
	EXPECT_NE( a.entry_point, "ps_tint" );
	EXPECT_EQ( a.entry_point, b.entry_point );
	EXPECT_NE( a.entry_point, c.entry_point );

	// ラッパーは uniform 以外の引数だけを受け取り、元の関数に値と一緒に渡す
	const size_t w = r.shader_source.find( a.entry_point + "(" );
	ASSERT_NE( w, std::string::npos );

	const std::string wrapper = r.shader_source.substr( w, r.shader_source.find( "}", w ) - w );
	EXPECT_NE( wrapper.find( "float4 p : SV_Position" ), std::string::npos );
	EXPECT_EQ( wrapper.find( "uniform" ), std::string::npos );
	EXPECT_NE( wrapper.find( ": SV_Target" ), std::string::npos );
	EXPECT_NE( wrapper.find( "return ps_tint( ( true ), p, ( 2.0f ) );" ), std::string::npos );
}

TEST( EffectParserTest, ReadsLineDirectiveForLocation )
{
	const auto r = parse( "#line 10 \"common.hlsl\"\ntechnique11 t { pass p { Foo = 1; } }" );

	ASSERT_FALSE( r.warnings.empty() );
	EXPECT_NE( r.warnings[ 0 ].find( "common.hlsl(10)" ), std::string::npos );
}

TEST( EffectParserTest, ParsesNumbers )
{
	double v = 0;

	EXPECT_TRUE( parse_number( "2.f", v ) ); EXPECT_DOUBLE_EQ( v, 2.0 );
	EXPECT_TRUE( parse_number( "0xFFFFFFFF", v ) ); EXPECT_DOUBLE_EQ( v, 4294967295.0 );
	EXPECT_TRUE( parse_number( "-1", v ) ); EXPECT_DOUBLE_EQ( v, -1.0 );
	EXPECT_TRUE( parse_number( "0.5", v ) ); EXPECT_DOUBLE_EQ( v, 0.5 );
	EXPECT_FALSE( parse_number( "ALL", v ) );

	std::array< float, 4 > c { };
	EXPECT_TRUE( parse_float4( "float4( 1.f, 0.5f, 0, 1 )", c ) );
	EXPECT_FLOAT_EQ( c[ 1 ], 0.5f );
	EXPECT_FALSE( parse_float4( "float3( 1, 2, 3 )", c ) );
}

// ----------------------------------------
// 状態の値の変換
// ----------------------------------------

namespace {

StateBlockDesc make_block( StateKind kind, std::vector< std::pair< std::string, std::string > > list )
{
	StateBlockDesc block;
	block.kind = kind;
	block.name = "test";

	for ( auto& [ key, value ] : list )
	{
		StateAssignment a;
		a.key = key;
		a.value = value;
		block.assignments.push_back( a );
	}

	return block;
}

} // namespace

TEST( EffectStateDescTest, BlendState )
{
	auto block = make_block( StateKind::BLEND, { { "BlendEnable", "True" }, { "SrcBlend", "SRC_ALPHA" }, { "DestBlend", "INV_SRC_ALPHA" } } );
	block.assignments[ 0 ].index = 0;

	std::vector< std::string > warnings;
	const auto d = direct_3d_11::make_blend_desc( block, warnings );

	EXPECT_TRUE( warnings.empty() );
	EXPECT_TRUE( d.RenderTarget[ 0 ].BlendEnable );
	EXPECT_EQ( d.RenderTarget[ 0 ].SrcBlend, D3D11_BLEND_SRC_ALPHA );
	EXPECT_EQ( d.RenderTarget[ 0 ].DestBlend, D3D11_BLEND_INV_SRC_ALPHA );

	// 書いていないものは FX11 の既定値
	EXPECT_EQ( d.RenderTarget[ 0 ].SrcBlendAlpha, D3D11_BLEND_ONE );
	EXPECT_EQ( d.RenderTarget[ 0 ].DestBlendAlpha, D3D11_BLEND_ZERO );
	EXPECT_EQ( d.RenderTarget[ 0 ].RenderTargetWriteMask, D3D11_COLOR_WRITE_ENABLE_ALL );
	EXPECT_FALSE( d.RenderTarget[ 1 ].BlendEnable );
}

TEST( EffectStateDescTest, DepthStencilAndRasterizerState )
{
	std::vector< std::string > warnings;

	const auto ds = direct_3d_11::make_depth_stencil_desc( make_block( StateKind::DEPTH_STENCIL, { { "DepthWriteMask", "ZERO" }, { "DepthFunc", "LESS_EQUAL" } } ), warnings );
	EXPECT_TRUE( ds.DepthEnable );
	EXPECT_EQ( ds.DepthWriteMask, D3D11_DEPTH_WRITE_MASK_ZERO );
	EXPECT_EQ( ds.DepthFunc, D3D11_COMPARISON_LESS_EQUAL );

	const auto rs = direct_3d_11::make_rasterizer_desc( make_block( StateKind::RASTERIZER, { { "CullMode", "None" }, { "FILLMODE", "WIREFRAME" }, { "DepthBias", "8.f" }, { "SlopeScaledDepthBias", "2.f" }, { "MultisampleEnable", "True" } } ), warnings );
	EXPECT_EQ( rs.CullMode, D3D11_CULL_NONE );
	EXPECT_EQ( rs.FillMode, D3D11_FILL_WIREFRAME );
	EXPECT_EQ( rs.DepthBias, 8 );
	EXPECT_FLOAT_EQ( rs.SlopeScaledDepthBias, 2.f );
	EXPECT_TRUE( rs.MultisampleEnable );
	EXPECT_TRUE( rs.DepthClipEnable );

	EXPECT_TRUE( warnings.empty() );
}

TEST( EffectStateDescTest, SamplerState )
{
	std::vector< std::string > warnings;

	const auto s = direct_3d_11::make_sampler_desc( make_block( StateKind::SAMPLER, { { "Filter", "ANISOTROPIC" }, { "AddressU", "BORDER" }, { "BorderColor", "float4( 1.f, 1.f, 1.f, 1.f )" }, { "MaxAnisotropy", "2.f" }, { "MaxLOD", "0" } } ), warnings );

	EXPECT_TRUE( warnings.empty() );
	EXPECT_EQ( s.Filter, D3D11_FILTER_ANISOTROPIC );
	EXPECT_EQ( s.AddressU, D3D11_TEXTURE_ADDRESS_BORDER );
	EXPECT_EQ( s.AddressV, D3D11_TEXTURE_ADDRESS_CLAMP );
	EXPECT_FLOAT_EQ( s.BorderColor[ 0 ], 1.f );
	EXPECT_EQ( s.MaxAnisotropy, 2u );
	EXPECT_FLOAT_EQ( s.MaxLOD, 0.f );

	// 書いていなければ FX11 の既定値（MaxAnisotropy 16）
	const auto d = direct_3d_11::make_sampler_desc( make_block( StateKind::SAMPLER, { } ), warnings );
	EXPECT_EQ( d.MaxAnisotropy, 16u );
}

TEST( EffectStateDescTest, ReportsUnknownKeyAndBadValue )
{
	std::vector< std::string > warnings;

	direct_3d_11::make_rasterizer_desc( make_block( StateKind::RASTERIZER, { { "CullMod", "NONE" }, { "FillMode", "DOTTED" } } ), warnings );

	ASSERT_EQ( warnings.size(), 2u );
	EXPECT_TRUE( contains( warnings, "CullMod" ) );
	EXPECT_TRUE( contains( warnings, "DOTTED" ) );
}

// ----------------------------------------
// 実際のシェーダー（media/shader/main.fx）
// ----------------------------------------

TEST( EffectParserTest, ParsesAndCompilesMainFx )
{
	const std::string path = ( std::filesystem::path( __FILE__ ).parent_path() / "../../media/shader/main.fx" ).lexically_normal().string();

	const std::string source = direct_3d_11::Effect::preprocess( path.c_str() );
	const auto r = parse( source );

	EXPECT_TRUE( r.errors.empty() ) << ( r.errors.empty() ? "" : r.errors[ 0 ] );
	EXPECT_TRUE( r.warnings.empty() ) << ( r.warnings.empty() ? "" : r.warnings[ 0 ] );

	// エンジンが名前で引く technique がそろっている
	for ( const char* name : { "main", "skin", "sprite", "sprite_color", "sprite_premultiplied", "msdf_text", "drawing_point", "drawing_line", "point_sprite", "main2d" } )
	{
		EXPECT_NE( r.find_technique( name ), nullptr ) << name;
	}

	// 状態の値に知らないキー・読めない値が無い
	std::vector< std::string > warnings;

	for ( const auto& block : r.state_blocks )
	{
		switch ( block.kind )
		{
			case StateKind::BLEND: direct_3d_11::make_blend_desc( block, warnings ); break;
			case StateKind::DEPTH_STENCIL: direct_3d_11::make_depth_stencil_desc( block, warnings ); break;
			case StateKind::RASTERIZER: direct_3d_11::make_rasterizer_desc( block, warnings ); break;
			case StateKind::SAMPLER: direct_3d_11::make_sampler_desc( block, warnings ); break;
		}
	}

	EXPECT_TRUE( warnings.empty() ) << ( warnings.empty() ? "" : warnings[ 0 ] );

	// パスが参照する状態がすべて定義されている
	for ( const auto& t : r.techniques )
	{
		for ( const auto& p : t.passes )
		{
			if ( ! p.blend_state.name.empty() ) { EXPECT_NE( r.find_state_block( StateKind::BLEND, p.blend_state.name ), nullptr ) << p.blend_state.name; }
			if ( ! p.depth_stencil_state.name.empty() ) { EXPECT_NE( r.find_state_block( StateKind::DEPTH_STENCIL, p.depth_stencil_state.name ), nullptr ) << p.depth_stencil_state.name; }
			if ( ! p.rasterizer_state.name.empty() ) { EXPECT_NE( r.find_state_block( StateKind::RASTERIZER, p.rasterizer_state.name ), nullptr ) << p.rasterizer_state.name; }
		}
	}

	// すべてのシェーダーが（デバイス無しで）警告なくコンパイルできる
	std::set< std::string > done;

	for ( const auto& t : r.techniques )
	{
		for ( const auto& p : t.passes )
		{
			for ( const auto& s : p.shaders )
			{
				if ( ! s || s->is_null() || ! done.insert( s->key() ).second )
				{
					continue;
				}

				std::string message;
				const auto code = direct_3d_11::Effect::compile_shader( r.shader_source, path.c_str(), *s, message );

				EXPECT_TRUE( code != nullptr ) << s->key() << "\n" << message;
				EXPECT_TRUE( message.empty() ) << s->key() << "\n" << message;
			}
		}
	}

	EXPECT_GT( done.size(), 0u );
}
