#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

/**
 * effect ファイル（.fx）の書式を読むパーサー
 *
 * FX11（開発が止まっている）をやめて自前で technique を組み立てるために、
 * technique11 / pass / Set… と、状態のブロック（BlendState 名前 { キー = 値; … }）を読む。
 * D3D には依存しない（テストで動かせるように）。シェーダーのコンパイルと状態オブジェクトの作成は Direct3D11/Effect が行う。
 */
namespace core::graphics::effect
{

/// シェーダーのステージ（並びは EffectPass が使う）
enum class ShaderStage { VS, HS, DS, GS, PS };

constexpr size_t SHADER_STAGE_COUNT = 5;

/// パスが使うシェーダー 1 つ（CompileShader( プロファイル, 関数( 引数… ) )）
struct ShaderRef
{
	std::string profile;					///< "vs_4_0" など
	std::string function;					///< 関数名。空なら NULL（そのステージのシェーダーを外す）
	std::vector< std::string > arguments;	///< uniform 引数に渡す式
	std::string entry_point;				///< コンパイルするときのエントリ関数（引数付きは生成したラッパーの名前）

	bool is_null() const { return function.empty(); }

	/// 同じシェーダーを共有するためのキー（関数名＋引数＋プロファイル）
	std::string key() const;
};

/// パスが指定した状態の名前
struct StateRef
{
	bool is_set = false;	///< false なら指定なし（前の状態のまま。FX11 と同じ）
	std::string name;		///< 空なら NULL（D3D の既定の状態）
};

struct PassDesc
{
	std::string name;

	/// nullopt なら指定なし（そのステージは触らない）
	std::array< std::optional< ShaderRef >, SHADER_STAGE_COUNT > shaders;

	StateRef blend_state;
	std::array< float, 4 > blend_factor { };
	uint32_t sample_mask = 0xFFFFFFFF;

	StateRef depth_stencil_state;
	uint32_t stencil_ref = 0;

	StateRef rasterizer_state;
};

struct TechniqueDesc
{
	std::string name;
	std::vector< PassDesc > passes;
};

enum class StateKind { BLEND, DEPTH_STENCIL, RASTERIZER, SAMPLER };

/// 状態ブロックの 1 行（キー[ index ] = 値;）
struct StateAssignment
{
	std::string key;
	int index = -1;			///< 添字が無ければ -1
	std::string value;		///< 値の式（そのままの文字列）
	std::string location;	///< 書かれていた場所（ファイル名(行)）。ログ用
};

struct StateBlockDesc
{
	StateKind kind = StateKind::BLEND;
	std::string name;
	std::vector< StateAssignment > assignments;
	std::string location;
};

struct ParseResult
{
	std::vector< TechniqueDesc > techniques;
	std::vector< StateBlockDesc > state_blocks;

	/// シェーダーをコンパイルするためのソース
	/// （technique と Blend / DepthStencil / Rasterizer のブロックを空白にし、SamplerState は宣言だけを残し、引数付きのエントリのラッパーを足したもの。行の位置は変えない）
	std::string shader_source;

	std::vector< std::string > warnings;	///< 読めたが対応していない書式など（ログに出す）
	std::vector< std::string > errors;		///< 読めなかったもの（読み込みを失敗させる）

	const TechniqueDesc* find_technique( const std::string& name ) const;
	const StateBlockDesc* find_state_block( StateKind, const std::string& name ) const;
};

/// プリプロセス済みの effect のソースを読む
ParseResult parse( const std::string& source );

/// 大文字小文字を区別せずに比べる（effect のキーと値は大文字小文字を区別しない）
bool equals_ignore_case( const std::string&, const std::string& );

/// 数を読む（"2.f"・"0.5"・"0xFFFFFFFF"・"-1" など）。読めなければ false
bool parse_number( const std::string&, double& );

/// float4( a, b, c, d ) を読む。読めなければ false
bool parse_float4( const std::string&, std::array< float, 4 >& );

} // namespace core::graphics::effect
