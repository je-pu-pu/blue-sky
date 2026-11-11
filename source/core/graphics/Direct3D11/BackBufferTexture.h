#pragma once

#include "Direct3D11Common.h"
#include <core/graphics/BackBufferTexture.h>

namespace core::graphics::direct_3d_11
{
	class Direct3D11;

	/**
	 * 画面への描画先
	 *
	 * スワップチェーンのバッファは MSAA なし（フリップモデルの要件）。MSAA のときは同じ大きさの自前の描画先を持ち、
	 * resolve() でスワップチェーンのバッファへ解決する。MSAA なしのときはスワップチェーンのバッファへ直接描く。
	 */
	class BackBufferTexture : public core::graphics::BackBufferTexture
	{
	private:
		// テクスチャ・描画先のビューを持つところは Texture / RenderTargetTexture と似ているが、スワップチェーンへの解決があるので別にしている
		Direct3D11*							direct_3d_;
		com_ptr< ID3D11Texture2D >			texture_2d_;				///< 描画先（MSAA のときは自前のテクスチャ、それ以外はスワップチェーンのバッファ）
		com_ptr< ID3D11Texture2D >			swap_chain_buffer_;			///< スワップチェーンのバッファ
		com_ptr< ID3D11RenderTargetView >	render_target_view_;		///< シェーダーから利用するための View
		D3D11_TEXTURE2D_DESC				texture_2d_desc_{};			///< 2D テクスチャについての説明

	public:
		BackBufferTexture( Direct3D11* d3d, IDXGISwapChain* swap_chain, const DXGI_SAMPLE_DESC& sample );

		/// 描画先のテクスチャ（画面の保存に使う。MSAA のこともある）
		ID3D11Texture2D* get_texture_2d() const { return texture_2d_.get(); }

		/// MSAA の描画先をスワップチェーンのバッファへ解決する（表示の直前に呼ぶ。MSAA なしなら何もしない）
		void resolve( ID3D11DeviceContext* context );

		ID3D11RenderTargetView* get_render_target_view() const { return render_target_view_.get(); }

		uint_t get_multi_sample_count() const { return texture_2d_desc_.SampleDesc.Count; }
		uint_t get_multi_sample_quality() const { return texture_2d_desc_.SampleDesc.Quality; }

		void activate() override;
		void clear( const Color& ) override;
	};

} // namespace core::graphics::direct_3d_11
