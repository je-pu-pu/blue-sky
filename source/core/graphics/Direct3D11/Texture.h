#pragma once

#include "Direct3D11.h"
#include <game/Texture.h>

namespace core::graphics::direct_3d_11
{

/**
 * Direct3D 11 Texture
 *
 */
class Texture : public game::Texture
{
private:
	Direct3D11* direct_3d_;
	com_ptr< ID3D11ShaderResourceView > view_;
	com_ptr< ID3D11Texture2D > texture_2d_;
	D3D11_TEXTURE2D_DESC texture_2d_desc_;

	Texture()
	{ }

public:
	Texture( Direct3D11* direct_3d, ID3D11ShaderResourceView* view )
		: direct_3d_( direct_3d )
		, view_( view )
	{
		ID3D11Resource* resource;
		view->GetResource( & resource );

		texture_2d_.reset( static_cast< ID3D11Texture2D* >( resource ) );
		texture_2d_->GetDesc( & texture_2d_desc_ );
	}

	virtual ~Texture() { }

	uint_t get_width() const override { return texture_2d_desc_.Width; }
	uint_t get_height() const override { return texture_2d_desc_.Height; }

	void bind_to_ds( uint_t slot ) const override { direct_3d_->bind_texture_to_ds( slot, this ); }
	void bind_to_ps( uint_t slot ) const override { direct_3d_->bind_texture_to_ps( slot, this ); }

	ID3D11ShaderResourceView* get_shader_resource_view() const { return view_.get(); }
	ID3D11Texture2D* get_texture_2d() const { return texture_2d_.get(); }

}; // class Texture

} // namespace core::graphics::direct_3d_11
