#pragma once

#include <blue_sky/graphics/Direct3D11/ShaderResource.h>
#include "Direct3D11Texture.h"

#include <common/auto_ptr.h>

#include <d3d11.h>

#include <vector>

class Direct3D11;

/**
 * Direct3D 11 によるシャドウマップ
 *
 */
class Direct3D11ShadowMap
{
public:
	static const int MaxCascadeLevels = 4;

	typedef Vector								Vector;
	typedef Matrix								Matrix;
	typedef Matrix								MatrixList[ MaxCascadeLevels ];

	typedef D3D11_VIEWPORT						Viewport;
	typedef std::vector< Viewport >				ViewportList;

	struct ConstantBufferData
	{
		static const int SLOT = 10;

		Matrix shadow_view_projection[ 3 ];
		float_t view_depth_per_cascade_level[ 4 ];
	};

	typedef blue_sky::graphics::direct_3d_11::ShaderResource< ConstantBufferData > ConstantBuffer;

	static const int shader_resource_view_slot_ = 1;		/// !!!!!!!!!!!

private:
	Direct3D11*								direct_3d_;
	common::auto_ptr< ConstantBuffer >		constant_buffer_;
	common::auto_ptr< Direct3D11Texture >	texture_;

	int							cascade_levels_;			///< CSM のレベル数

	ID3D11Texture2D*			depth_stencil_texture_;
	ID3D11DepthStencilView*		depth_stencil_view_;

	ViewportList				viewport_list_;

	static Vector				light_position_;			///< ( 0, 0, 0 ) を向く sun

	static Matrix				view_matrix_;
	static MatrixList			projection_matrix_list_;

	static ConstantBufferData	constant_buffer_data_;		/// !!!

public:
	Direct3D11ShadowMap( Direct3D11*, int, size_t );
	virtual ~Direct3D11ShadowMap();

	void ready_to_render_shadow_map();
	void ready_to_render_shadow_map_with_cascade_level( int );
	// void finish_render_shadow_map();

	void ready_to_render_scene();

	int get_cascade_levels() const { return cascade_levels_; }
	
	void set_light_position( const Vector& );
	void set_eye_position( const Vector& );

	const ConstantBuffer* getConstantBuffer() const { return constant_buffer_.get(); }

	Direct3D11Texture* getTexture() { return texture_.get(); }
	const Direct3D11Texture* getTexture() const { return texture_.get(); }

	const Matrix getViewProjectionMatrix( int level ) const { return view_matrix_ * projection_matrix_list_[ level ]; }
	
}; // class Direct3D11ShadowMap
