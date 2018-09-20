#pragma once

#include <blue_sky/graphics/Line.h>
#include <d3d11.h>

namespace game
{
	class Texture;
}

namespace core::graphics::direct_3d_11
{
	class Direct3D11;
}

namespace blue_sky::graphics::direct_3d_11
{

/**
 * Direct3D11 Žè•`‚«•—ƒ‰ƒCƒ“
 *
 */
class Line : public blue_sky::graphics::Line
{
public:
	using Texture = game::Texture;
	using Direct3D = core::graphics::direct_3d_11::Direct3D11;

	static const DXGI_FORMAT IndexBufferFormat = DXGI_FORMAT_R16_UINT;

protected:
	Direct3D*		direct_3d_;

	ID3D11Buffer*	vertex_buffer_;
	ID3D11Buffer*	index_buffer_;
	
	Texture*		texture_;

	void create_vertex_buffer() override;
	void create_index_buffer() override;

	void create_texture() override;

public:
	explicit Line( Direct3D* );
	virtual ~Line();

	void render_part( int part_count = 99999 ) const;

}; // class Line

} // namespace blue_sky::graphics::direct_3d_11
