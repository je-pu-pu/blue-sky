#pragma once

#include "Texture.h"
#include "PixelFormat.h"

#include <core/graphics/RenderTargetTexture.h>

namespace core::graphics::direct_3d_11
{
	class Direct3D11;


	/**
	 * Direct3D 11 ƒŒƒ“ƒ_ƒŠƒ“ƒOŒ‹‰Ê‚ğ‘‚«‚Ş–‚ª‚Å‚«‚é Texture
	 *
	 */
	class RenderTargetTexture : public core::graphics::RenderTargetTexture
	{
	private:
		core::graphics::direct_3d_11::Texture texture_;
		com_ptr< ID3D11RenderTargetView > render_target_view_;

	protected:

	public:
		RenderTargetTexture( PixelFormat format );
		RenderTargetTexture( PixelFormat format, int width, int height );

		void bind_to_ds( uint_t slot ) const override { texture_.bind_to_ds( slot ); }
		void bind_to_ps( uint_t slot ) const override { texture_.bind_to_ps( slot ); }

		uint_t get_width() const override { return texture_.get_width(); };
		uint_t get_height() const override { return texture_.get_height(); };

		uint_t get_multi_sample_count() const override { return texture_.get_multi_sample_count(); };
		uint_t get_multi_sample_quality() const override { return texture_.get_multi_sample_quality(); };

		ID3D11RenderTargetView* get_render_target_view() { return render_target_view_.get(); }

		void activate() override;

		// void clear( const game::Color& ) override;

	}; // class RenderTargetTexture

} // namespace core::graphics::direct_3d_11
