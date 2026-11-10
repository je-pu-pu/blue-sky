#pragma once

#include <blue_sky/graphics/Mesh.h>
#include <core/DirectX.h>
#include <d3d11.h>

namespace core::graphics
{
	class GraphicsManager;
}

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
	const core::graphics::GraphicsManager*	graphics_manager_;	///< 描画回数を数える先（作り手が渡す。以前は core::Service から取っていた）
	BufferList			vertex_buffer_list_;
	BufferList			index_buffer_list_;

protected:
	void create_vertex_buffer() override;
	void create_index_buffer() override;

	void update_vertex_buffer() override;
	void update_index_buffer() override;

public:
	MeshBuffer( Direct3D* d3d, Type type, const core::graphics::GraphicsManager* graphics_manager )
		: Buffer( type )
		, direct_3d_( d3d )
		, graphics_manager_( graphics_manager )
	{

	}
	
	~MeshBuffer() override
	{

	}

	void bind() const override;
	void render( uint_t ) const override;
	void render_instanced( uint_t n, uint_t instance_count ) const override;

}; // class MeshBuffer

} // namespace blue_sky::graphics::direct_3d_11
