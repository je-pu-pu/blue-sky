#pragma once

#include "Texture.h"
#include "PixelFormat.h"

#include <core/graphics/RenderTargetTexture.h>
#include <core/graphics/RenderTargetTextureOptions.h>

#include <memory>

namespace core::graphics::direct_3d_11
{
	class Direct3D11;


	/**
	 * Direct3D 11 レンダリング結果を書き込む事ができる Texture
	 *
	 */
	class RenderTargetTexture : public core::graphics::RenderTargetTexture
	{
	public:
		using Options = core::graphics::RenderTargetTextureOptions;

	private:
		Direct3D11* direct_3d_;		///< テクスチャを作る・描画先にする先（作り手が渡す。以前は Direct3D11 の get_instance() から取っていた）
		PixelFormat format_;
		Options options_;			///< ミップの段数・MSAA（作れる形に直したもの。作り直すときも同じ指定で作る）
		bool is_screen_sized_;
		std::unique_ptr< core::graphics::direct_3d_11::Texture > texture_;
		std::unique_ptr< core::graphics::direct_3d_11::Texture > resolved_;		///< MSAA を解決した先（読むときに作る）
		com_ptr< ID3D11RenderTargetView > render_target_view_;

		void create_texture( int width, int height );
		void recreate_if_needed();

	protected:

	public:
		RenderTargetTexture( Direct3D11* direct_3d, PixelFormat format );
		RenderTargetTexture( Direct3D11* direct_3d, PixelFormat format, const Options& options );
		RenderTargetTexture( Direct3D11* direct_3d, PixelFormat format, int width, int height );

		/// このテクスチャを作った Direct3D11（中身を読み戻す所がデバイス・コンテキストを得るのに使う）
		Direct3D11* get_direct_3d() const { return direct_3d_; }

		void bind_to_ds( uint_t slot ) const override { texture_->bind_to_ds( slot ); }
		void bind_to_ps( uint_t slot ) const override { texture_->bind_to_ps( slot ); }

		uint_t get_width() const override { return texture_->get_width(); };
		uint_t get_height() const override { return texture_->get_height(); };

		uint_t get_multi_sample_count() const override { return texture_->get_multi_sample_count(); };
		uint_t get_multi_sample_quality() const override { return texture_->get_multi_sample_quality(); };

		ID3D11RenderTargetView* get_render_target_view() { return render_target_view_.get(); }

		core::graphics::Texture* resolve_for_sampling() override;

		void activate() override;
		void clear( const Color& ) override;

	}; // class RenderTargetTexture

} // namespace core::graphics::direct_3d_11
