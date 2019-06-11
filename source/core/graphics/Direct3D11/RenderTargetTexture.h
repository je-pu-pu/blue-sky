#pragma once

#include "Texture.h"
#include "PixelFormat.h"

namespace core::graphics::direct_3d_11
{
	/**
	 * Direct3D 11 レンダリング結果を書き込む事ができる Texture
	 *
	 */
	class RenderTargetTexture : public Texture
	{
	private:
		com_ptr< ID3D11RenderTargetView > render_target_view_;

	protected:
		RenderTargetTexture( Direct3D11* d3d )
			: Texture( d3d )
		{ }

		void create_render_target_view();

	public:
		RenderTargetTexture( Direct3D11* d3d, PixelFormat format, int width, int height );

		ID3D11RenderTargetView* get_render_target_view() const { return render_target_view_.get(); }

	}; // class RenderTargetTexture

} // namespace core::graphics::direct_3d_11
