#ifndef DIRECT_3D_11_TEXTURE_H
#define DIRECT_3D_11_TEXTURE_H

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
	com_ptr< ID3D11ShaderResourceView > view_;
	D3D11_TEXTURE2D_DESC texture_2d_desc_;

public:
	Direct3D11Texture( ID3D11ShaderResourceView* view )
		: view_( view )
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

	ID3D11ShaderResourceView* get_shader_resource_view() const { return view_.get(); }

}; // class Direct3D11Texture

#endif // DIRECT_3D_11_MATRIX_H
