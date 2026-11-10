#pragma once

#include <core/graphics/EffectPass.h>
#include <core/graphics/EffectParser.h>

#include <d3d11.h>
#include <wrl/client.h>

#include <array>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace core::graphics::direct_3d_11
{
	class Direct3D11;

/**
 * コンパイル済みのシェーダー 1 つ
 *
 * 同じ関数・引数・プロファイルのシェーダーは、パス同士で共有する。
 */
struct EffectShader
{
	template< typename T > using ComPtr = Microsoft::WRL::ComPtr< T >;

	effect::ShaderStage stage = effect::ShaderStage::VS;
	ComPtr< ID3D11DeviceChild > object;		///< ID3D11VertexShader など
	ComPtr< ID3DBlob > byte_code;			///< 入力レイアウトを作るのに使う

	/// このシェーダーが使うサンプラー（スロット → 状態）。スロットはリフレクションで調べたもの
	std::vector< std::pair< UINT, ComPtr< ID3D11SamplerState > > > sampler_list;
};

/**
 * パスが設定するもの（Effect が組み立てる）
 *
 * 指定の無いもの（has_… が false・シェーダーが nullopt）は apply() で触らない（FX11 と同じ。前の状態のまま）。
 */
struct EffectPassSetup
{
	template< typename T > using ComPtr = Microsoft::WRL::ComPtr< T >;

	/// nullopt : 触らない、nullptr : そのステージのシェーダーを外す
	std::array< std::optional< std::shared_ptr< const EffectShader > >, effect::SHADER_STAGE_COUNT > shader_list;

	bool has_blend_state = false;
	ComPtr< ID3D11BlendState > blend_state;		///< nullptr なら D3D の既定
	std::array< float, 4 > blend_factor { };
	UINT sample_mask = 0xFFFFFFFF;

	bool has_depth_stencil_state = false;
	ComPtr< ID3D11DepthStencilState > depth_stencil_state;
	UINT stencil_ref = 0;

	bool has_rasterizer_state = false;
	ComPtr< ID3D11RasterizerState > rasterizer_state;
};

/**
 * effect のパス（シェーダーと状態をまとめて設定する）
 *
 */
class EffectPass : public core::graphics::EffectPass
{
public:
	using Direct3D		= Direct3D11;
	using InputLayout	= ID3D11InputLayout;

private:
	Direct3D*			direct_3d_;
	EffectPassSetup		setup_;

public:
	EffectPass( Direct3D*, EffectPassSetup );
	~EffectPass();

	/// 頂点シェーダーのバイトコード（入力シグネチャ）から入力レイアウトを作る
	InputLayout* create_input_layout( D3D11_INPUT_ELEMENT_DESC[], UINT );

	void apply() const override;

}; // class EffectPass

} // namespace namespace core::graphics::direct_3d_11
