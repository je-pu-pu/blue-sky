#pragma once

#include <core/graphics/EffectParser.h>

#include <d3d11.h>

#include <string>
#include <vector>

/**
 * effect の状態ブロック（BlendState 名前 { キー = 値; … }）を D3D11 の desc に変換する
 *
 * デバイスは使わない（テストで動かせるように）。
 * 書かれていないキーは FX11 と同じ既定値にする（FX11 の EffectNonRuntime.cpp の SBlendBlock などのコンストラクタに合わせた）。
 * 知らないキーや読めない値は warnings に足す（黙って無視しない）。
 */
namespace core::graphics::direct_3d_11
{

D3D11_BLEND_DESC make_blend_desc( const effect::StateBlockDesc&, std::vector< std::string >& warnings );
D3D11_DEPTH_STENCIL_DESC make_depth_stencil_desc( const effect::StateBlockDesc&, std::vector< std::string >& warnings );
D3D11_RASTERIZER_DESC make_rasterizer_desc( const effect::StateBlockDesc&, std::vector< std::string >& warnings );
D3D11_SAMPLER_DESC make_sampler_desc( const effect::StateBlockDesc&, std::vector< std::string >& warnings );

} // namespace core::graphics::direct_3d_11
