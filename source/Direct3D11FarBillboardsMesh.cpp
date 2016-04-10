#include "Direct3D11FarBillboardsMesh.h"
#include "Direct3D11Material.h"
#include "DirectX.h"

#include <list>

#include "memory.h"

Direct3D11FarBillboardsMesh::Direct3D11FarBillboardsMesh( Direct3D* direct_3d )
	: Direct3D11Mesh( direct_3d )
{

}

void Direct3D11FarBillboardsMesh::create_index_buffer( Material* material )
{
	if ( material->get_index_list().empty() )
	{
		return;
	}

	struct Face
	{
		Material::Index index[ 3 ];
	};

	typedef std::list< Face > Facelist;
	Facelist face_list;

	for ( uint_t  n = 0; n < material->get_index_list().size() / 3; n++ )
	{
		Face f;
		f.index[ 0 ] = material->get_index_list()[ n * 3 + 0 ];
		f.index[ 1 ] = material->get_index_list()[ n * 3 + 1 ];
		f.index[ 2 ] = material->get_index_list()[ n * 3 + 2 ];

		face_list.push_back( f );
	}

	struct VertexLengthXzGreater
	{
	private:
		const Direct3D11Mesh* mesh_;

	public:
		VertexLengthXzGreater( const Direct3D11Mesh* mesh )
			: mesh_( mesh )
		{
		}

		bool operator () ( Face a, Face b )
		{
			return (
				DirectX::XMVectorGetX( DirectX::XMVector2Length( DirectX::XMVectorSet( mesh_->get_vertex_list()[ a.index[ 0 ] ].Position.x, mesh_->get_vertex_list()[ a.index[ 0 ] ].Position.z, 0, 0 ) ) ) +
				DirectX::XMVectorGetX( DirectX::XMVector2Length( DirectX::XMVectorSet( mesh_->get_vertex_list()[ a.index[ 1 ] ].Position.x, mesh_->get_vertex_list()[ a.index[ 1 ] ].Position.z, 0, 0 ) ) ) +
				DirectX::XMVectorGetX( DirectX::XMVector2Length( DirectX::XMVectorSet( mesh_->get_vertex_list()[ a.index[ 2 ] ].Position.x, mesh_->get_vertex_list()[ a.index[ 2 ] ].Position.z, 0, 0 ) ) )
			) >
			(
				DirectX::XMVectorGetX( DirectX::XMVector2Length( DirectX::XMVectorSet( mesh_->get_vertex_list()[ b.index[ 0 ] ].Position.x, mesh_->get_vertex_list()[ b.index[ 0 ] ].Position.z, 0, 0 ) ) ) +
				DirectX::XMVectorGetX( DirectX::XMVector2Length( DirectX::XMVectorSet( mesh_->get_vertex_list()[ b.index[ 1 ] ].Position.x, mesh_->get_vertex_list()[ b.index[ 1 ] ].Position.z, 0, 0 ) ) ) +
				DirectX::XMVectorGetX( DirectX::XMVector2Length( DirectX::XMVectorSet( mesh_->get_vertex_list()[ b.index[ 2 ] ].Position.x, mesh_->get_vertex_list()[ b.index[ 2 ] ].Position.z, 0, 0 ) ) )
			);
		}
	};

	face_list.sort( VertexLengthXzGreater( this ) );

	material->get_index_list().clear();

	for ( auto i = face_list.begin(); i != face_list.end(); ++i )
	{
		material->get_index_list().push_back( i->index[ 0 ] );
		material->get_index_list().push_back( i->index[ 1 ] );
		material->get_index_list().push_back( i->index[ 2 ] );
	}

	material->create_index_buffer();
}