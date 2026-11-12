#pragma once

#include <core/graphics/ConstantBuffer.h>
#include <core/graphics/Direct3D11/Direct3D11.h>

#include <cstring>

namespace core::graphics::direct_3d_11
{

/**
 * D3D11 の定数バッファを作る（大きさは 16 バイトの倍数に切り上げる）
 *
 * @param usage DEFAULT なら D3D11_USAGE_DEFAULT、DYNAMIC なら D3D11_USAGE_DYNAMIC（CPU から書き込める）
 */
inline ID3D11Buffer* create_d3d11_constant_buffer( Direct3D11* direct_3d, size_t size, core::graphics::ConstantBufferUsage usage )
{
	D3D11_BUFFER_DESC buffer_desc = { 0 };

	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.ByteWidth = static_cast< UINT >( core::graphics::ConstantBuffer::get_aligned_size( size ) );

	if ( usage == core::graphics::ConstantBufferUsage::DYNAMIC )
	{
		buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
		buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else
	{
		buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	}

	ID3D11Buffer* buffer = nullptr;
	DIRECT_X_FAIL_CHECK( direct_3d->getDevice()->CreateBuffer( & buffer_desc, 0, & buffer ) );

	return buffer;
}

/**
 * D3D11 の定数バッファの中身を data で置き換える
 *
 * DEFAULT は UpdateSubresource で、DYNAMIC は Map(WRITE_DISCARD) で書き込む。
 * DYNAMIC は捨てた後の中身が決まらないので、size から 16 バイトの倍数までの余りは 0 で埋める
 */
inline void update_d3d11_constant_buffer( Direct3D11* direct_3d, ID3D11Buffer* buffer, const void* data, size_t size, core::graphics::ConstantBufferUsage usage )
{
	auto* context = direct_3d->getImmediateContext();

	if ( usage == core::graphics::ConstantBufferUsage::DYNAMIC )
	{
		D3D11_MAPPED_SUBRESOURCE mapped = {};
		DIRECT_X_FAIL_CHECK( context->Map( buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, & mapped ) );

		std::memcpy( mapped.pData, data, size );
		std::memset( static_cast< char* >( mapped.pData ) + size, 0, core::graphics::ConstantBuffer::get_aligned_size( size ) - size );

		context->Unmap( buffer, 0 );
	}
	else
	{
		context->UpdateSubresource( buffer, 0, 0, data, 0, 0 );
	}
}

} // namespace core::graphics::direct_3d_11
