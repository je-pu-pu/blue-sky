#pragma once

#include "Direct3D11Common.h"
#include <core/graphics/BackBufferTexture.h>

namespace core::graphics::direct_3d_11
{
	class Direct3D11;

	/**
	 * Direct3D11 のバックバッファを参照するテクスチャ
	 *
	 */
	class BackBufferTexture : public core::graphics::BackBufferTexture
	{
	private:
		// @todo Texture と共通化する
		Direct3D11*							direct_3d_;
		com_ptr< ID3D11Texture2D >			texture_2d_;				///< 実際の 2D テクスチャデータ
		com_ptr< ID3D11RenderTargetView >	render_target_view_;		///< シェーダーから利用するための View
		D3D11_TEXTURE2D_DESC				texture_2d_desc_{};			///< 2D テクスチャについての説明

	public:
		BackBufferTexture( Direct3D11* d3d, IDXGISwapChain* swap_chain );

		ID3D11RenderTargetView* get_render_target_view() const { return render_target_view_.get(); }

		uint_t get_multi_sample_count() const { return texture_2d_desc_.SampleDesc.Count; }
		uint_t get_multi_sample_quality() const { return texture_2d_desc_.SampleDesc.Quality; }

		void activate() override;
	};

} // namespace core::graphics::direct_3d_11
