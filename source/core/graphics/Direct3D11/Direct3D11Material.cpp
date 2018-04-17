#include "Direct3D11Material.h"
#include "Direct3D11TextureManager.h"
#include "Direct3D11.h"

#include <common/exception.h>
#include <common/math.h>

Direct3D11Material::Direct3D11Material( Direct3D11* direct_3d )
	: direct_3d_( direct_3d )
	, index_buffer_( 0 )
	, texture_( 0 )
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
 * �V�X�e����������̃C���f�b�N�X�����N���A����
 *
 */
void Direct3D11Material::clear_index_list()
{
	index_list_.clear();
}

/**
 * �e�N�X�`����ǂݍ���
 *
 * @param file_name �e�N�X�`���t�@�C����
 */
void Direct3D11Material::load_texture( const char* file_name )
{
	texture_ = direct_3d_->getTextureManager()->load( file_name, file_name );
}

const game::Texture* Direct3D11Material::get_texture() const
{
	return texture_;
}

void Direct3D11Material::set_texture( const game::Texture* texture )
{
	texture_ = texture;
}

void Direct3D11Material::bind_to_ia() const
{
	direct_3d_->getImmediateContext()->IASetIndexBuffer( index_buffer_, IndexBufferFormat, 0 );
}

void Direct3D11Material::render() const
{
	direct_3d_->getImmediateContext()->DrawIndexed( index_count_, 0, 0 );
}