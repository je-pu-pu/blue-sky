#include "Direct3D11Material.h"
#include "Direct3D11TextureManager.h"
#include "Direct3D11.h"
#include "DirectX.h"

#include <common/exception.h>
#include <common/math.h>

Direct3D11Material::Direct3D11Material( Direct3D11* direct_3d )
	: direct_3d_( direct_3d )
	, index_buffer_( 0 )
	, texture_resource_view_( 0 )
	, index_count_( 0 )
{

}

Direct3D11Material::~Direct3D11Material()
{
	DIRECT_X_RELEASE( index_buffer_ );
}

void Direct3D11Material::create_index_buffer()
{
	if ( index_list_.empty() )
	{
		return;
	}

	D3D11_BUFFER_DESC buffer_desc = { 0 };

	buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    buffer_desc.ByteWidth = sizeof( Index ) * index_list_.size();

	D3D11_SUBRESOURCE_DATA data = { 0 };
	data.pSysMem = & index_list_[ 0 ];

	DIRECT_X_FAIL_CHECK( direct_3d_->getDevice()->CreateBuffer( & buffer_desc, & data, & index_buffer_ ) );

	index_count_ = index_list_.size();
}

/**
 * システムメモリ上のインデックス情報をクリアする
 *
 */
void Direct3D11Material::clear_index_list()
{
	index_list_.clear();
}

void Direct3D11Material::load_texture( const char* file_name )
{
	std::string texture_file_name = file_name; // boost::filesystem::basename( boost::filesystem::path( file_name ) ) + ".png";
	texture_resource_view_ = direct_3d_->getTextureManager()->load( texture_file_name.c_str(), texture_file_name.c_str() )->get_shader_resource_view();
}

void Direct3D11Material::render() const
{
	direct_3d_->getImmediateContext()->IASetIndexBuffer( index_buffer_, IndexBufferFormat, 0 );
	direct_3d_->getImmediateContext()->PSSetShaderResources( 0, 1, & texture_resource_view_ );

	// ID3D11ShaderResourceView* text_view = direct_3d_->getTextView();
	// direct_3d_->getImmediateContext()->PSSetShaderResources( 0, 1, & text_view );

	direct_3d_->getImmediateContext()->DrawIndexed( index_count_, 0, 0 );
}