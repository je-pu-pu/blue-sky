#ifndef DIRECT_3D_11_SHADOW_MAP_H
#define DIRECT_3D_11_SHADOW_MAP_H

#include "Direct3D11ConstantBuffer.h"

#include "type.h"

#include <common/auto_ptr.h>

#include <d3d11.h>
#include <xnamath.h>

#include <vector>

class Direct3D11;

using common::auto_ptr;

/**
 * Direct3D 11 によるシャドウマップ
 *
 */
class Direct3D11ShadowMap
{
public:
	static const int MaxCascadeLevels = 4;

	typedef XMVECTOR							Vector;

	typedef XMMATRIX							Matrix;
	typedef Matrix								MatrixList[ MaxCascadeLevels ];

	typedef D3D11_VIEWPORT						Viewport;
	typedef std::vector< Viewport >				ViewportList;

	struct ConstantBufferData
	{
		static const int DEFAULT_SLOT = 10;

		XMMATRIX shadow_view_projection[ 3 ];
		float_t view_depth_per_cascade_level[ 4 ];
	};

	typedef Direct3D11ConstantBuffer< ConstantBufferData > ConstantBuffer;

	static const int shader_resource_view_slot_ = 1;		/// !!!!!!!!!!!

private:
	Direct3D11*					direct_3d_;
	auto_ptr< ConstantBuffer >	constant_buffer_;

	bool						enabled_;					///< 有効フラグ
	int							cascade_levels_;			///< CSM のレベル数

	ID3D11Texture2D*			depth_stencil_texture_;
	ID3D11DepthStencilView*		depth_stencil_view_;

	ID3D11ShaderResourceView*	shader_resource_view_;

	ViewportList				viewport_list_;

	static Vector				light_position_;			//!< ( 0, 0, 0 ) を向く sun

	static Matrix				view_matrix_;
	static MatrixList			projection_matrix_list_;

	static ConstantBufferData	constant_buffer_data_;		/// !!!

public:
	Direct3D11ShadowMap( Direct3D11*, int, size_t );
	virtual ~Direct3D11ShadowMap();

	void ready_to_render_shadow_map();
	void ready_to_render_shadow_map_with_cascade_level( int );

	void ready_to_render_scene();

	void set_enabled( bool e ) { enabled_ = e; }
	bool is_enabled() const { return enabled_; }

	int get_cascade_levels() const { return cascade_levels_; }

	ID3D11ShaderResourceView* getShaderResourceView() { return shader_resource_view_; }

	void setLightPosition( const Vector& );
	void setEyePosition( const XMVECTOR& );

	const ConstantBuffer* getConstantBuffer() const { return constant_buffer_.get(); }

	const Matrix getViewProjectionMatrix( int level ) const { return view_matrix_ * projection_matrix_list_[ level ]; }
	
}; // class Direct3D11ShadowMap

#endif // DIRECT_3D_11_SHADOW_MAP_H