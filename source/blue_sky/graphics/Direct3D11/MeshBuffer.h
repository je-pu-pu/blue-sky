#pragma once

#include <blue_sky/graphics/Mesh.h>
#include <core/DirectX.h>
#include <d3d11.h>

namespace core::graphics::direct_3d_11
{
	class Direct3D11;
}

namespace blue_sky::graphics::direct_3d_11
{

/**
 * Direct3D11 メッシュバッファ
 *
 */
class MeshBuffer : public blue_sky::graphics::Mesh::Buffer
{
public:
	using Direct3D = core::graphics::direct_3d_11::Direct3D11;
	using BufferList = std::vector< com_ptr< ID3D11Buffer > >;

	static const DXGI_FORMAT IndexBufferFormat = DXGI_FORMAT_R16_UINT;

private:
	Direct3D*			direct_3d_;
	BufferList			vertex_buffer_list_;
	BufferList			index_buffer_list_;

protected:
	void create_vertex_buffer() override;
	void create_index_buffer() override;

	void update_vertex_buffer() override;
	void update_index_buffer() override;

public:
	explicit MeshBuffer( Direct3D* d3d, Type type )
		: Buffer( type )
		, direct_3d_( d3d )
	{

	}
	
	~MeshBuffer()
	{

	}

	void bind() const override;
	void render( uint_t ) const override;

}; // class MeshBuffer

} // namespace blue_sky::graphics::direct_3d_11
