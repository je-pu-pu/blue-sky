#pragma once

#include "Texture.h"
#include "PixelFormat.h"

#include <core/graphics/RenderTargetTexture.h>

namespace core::graphics::direct_3d_11
{
	class Direct3D11;

	/*
	namespace detail
	{
		class RenderTargetView
		{
		private:
			com_ptr< ID3D11RenderTargetView > render_target_view_;

		protected:
			void create_render_target_view()
			{
				// DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateRenderTargetView( texture_2d_.get(), nullptr, & render_target_view_ ) );
			}

		public:
			ID3D11RenderTargetView* get_render_target_view() const { return render_target_view_.get(); }
		};
	}
	/*

	/**
	 * Direct3D 11 ƒŒƒ“ƒ_ƒŠƒ“ƒOŒ‹‰Ê‚ğ‘‚«‚Ş–‚ª‚Å‚«‚é Texture
	 *
	 */
	class RenderTargetTexture : public core::graphics::RenderTargetTexture
	{
	private:
		Texture texture_;
		com_ptr< ID3D11RenderTargetView > render_target_view_;

	protected:

	public:
		RenderTargetTexture( Direct3D11* d3d, PixelFormat format, int width, int height );

		void bind_to_ds( uint_t slot ) const override { texture_.bind_to_ds( slot ); }
		void bind_to_ps( uint_t slot ) const override { texture_.bind_to_ps( slot ); }

		ID3D11RenderTargetView* get_render_target_view() { return render_target_view_.get(); }

		// override { return }

		// void activate() override;

		// void clear( const game::Color& ) override;

	}; // class RenderTargetTexture

} // namespace core::graphics::direct_3d_11
