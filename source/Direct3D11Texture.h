#ifndef DIRECT_3D_11_TEXTURE_H
#define DIRECT_3D_11_TEXTURE_H

#include "Direct3D11.h"
#include "DirectX.h"
#include <game/Texture.h>
#include <d3d11.h>

/**
 * Direct3D 11 Texture
 *
 */
class Direct3D11Texture : public game::Texture
{
private:
	Direct3D11* direct_3d_;
	com_ptr< ID3D11ShaderResourceView > view_;
	D3D11_TEXTURE2D_DESC texture_2d_desc_;

	Direct3D11Texture()
	{ }

public:
	Direct3D11Texture( Direct3D11* direct_3d, ID3D11ShaderResourceView* view )
		: direct_3d_( direct_3d )
		, view_( view )
	{
		com_ptr< ID3D11Resource > texture_2d_resource;
		view->GetResource( & texture_2d_resource );

		ID3D11Texture2D* texture_2d = static_cast< ID3D11Texture2D* >( texture_2d_resource.get() );
		texture_2d->GetDesc( & texture_2d_desc_ );
	}

	~Direct3D11Texture()
	{

	}

	uint_t get_width() const { return texture_2d_desc_.Width; }
	uint_t get_height() const { return texture_2d_desc_.Height; }

	void bind_to_ps( uint_t slot ) const
	{
		direct_3d_->bind_texture_to_ps( slot, this );
	}

	ID3D11ShaderResourceView* get_shader_resource_view() const { return view_.get(); }

}; // class Direct3D11Texture

#endif // DIRECT_3D_11_MATRIX_H
