#pragma once

#include <blue_sky/graphics/Line.h>
#include <core/graphics/Texture.h>
#include <d3d11.h>

namespace core::graphics::direct_3d_11
{
	class Direct3D11;
}

namespace blue_sky::graphics
{
	class GraphicsManager;
}

namespace blue_sky::graphics::direct_3d_11
{

/**
 * Direct3D11 手描き風ライン
 *
 */
class Line : public blue_sky::graphics::Line
{
public:
	using Texture = core::graphics::Texture;
	using Direct3D = core::graphics::direct_3d_11::Direct3D11;

	static const DXGI_FORMAT IndexBufferFormat = DXGI_FORMAT_R16_UINT;

protected:
	Direct3D*		direct_3d_;
	blue_sky::graphics::GraphicsManager*	graphics_manager_;	///< テクスチャの読み込み先・描画回数を数える先（作り手が渡す。以前は core::Service から取っていた）

	ID3D11Buffer*	vertex_buffer_;
	ID3D11Buffer*	index_buffer_;
	
	core::graphics::TextureHandle	texture_;		///< 線の画像（ハンドルで持ち、描くたびに引く。以前は生ポインタで、解放の後に古いものを指し得た）
	mutable bool	texture_lost_logged_ = false;	///< 画像が引けなかったログを出したか（毎フレーム出さない）

	void create_vertex_buffer() override;
	void create_index_buffer() override;

	void create_texture() override;

public:
	Line( Direct3D*, blue_sky::graphics::GraphicsManager* );
	virtual ~Line() override;

	void render_part( int part_count = 99999 ) const;
	void render_instanced( uint_t instance_count ) const override;

}; // class Line

} // namespace blue_sky::graphics::direct_3d_11
