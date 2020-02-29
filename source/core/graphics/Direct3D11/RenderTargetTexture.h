#pragma once

#include "Texture.h"
#include "PixelFormat.h"

namespace core::graphics::direct_3d_11
{
	namespace detail
	{
		class RenderTargetView
		{
		private:
			com_ptr< ID3D11RenderTargetView > render_target_view_;

		protected:
			void create_render_target_view()
			{
				DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateRenderTargetView( texture_2d_.get(), nullptr, & render_target_view_ ) );
			}

		public:
			ID3D11RenderTargetView* get_render_target_view() const { return render_target_view_.get(); }
		};
	}

	/**
	 * Direct3D 11 ƒŒƒ“ƒ_ƒŠƒ“ƒOŒ‹‰Ê‚ğ‘‚«‚Ş–‚ª‚Å‚«‚é Texture
	 *
	 */
	class RenderTargetTexture : public game::RenderTargetTexture, public detail::RenderTargetView
	{
	private:
		Direct3D11* direct_3d_;
		com_ptr< ID3D11RenderTargetView > render_target_view_;

	protected:
		RenderTargetTexture( Direct3D11* d3d )
			: direct_3d_( d3d )
		{ }

		

	public:
		RenderTargetTexture( Direct3D11* d3d, PixelFormat format, int width, int height );

		// void activate() override;

		// void clear( const game::Color& ) override;

	}; // class RenderTargetTexture

} // namespace core::graphics::direct_3d_11
