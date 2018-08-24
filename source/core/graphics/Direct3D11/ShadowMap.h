#pragma once

#include "Direct3D11Texture.h"
#include <core/graphics/Direct3D11/ShaderResource.h>
#include <core/graphics/ShadowMap.h>
#include <d3d11.h>
#include <vector>

class Direct3D11;

namespace core::graphics::direct_3d_11
{

/**
 * Direct3D 11 シャドウマップ
 *
 */
class ShadowMap : public graphics::ShadowMap
{
public:
	static const int MaxCascadeLevels = 4;

	using Vector		= Vector;
	using Matrix		= Matrix;
	using MatrixList	= Matrix[ MaxCascadeLevels ];

	using Viewport		= D3D11_VIEWPORT;
	using ViewportList	= std::vector< Viewport >;

	struct ShaderResourceData
	{
		static const int SLOT = 10;

		Matrix shadow_view_projection[ 3 ];
		float_t view_depth_per_cascade_level[ 4 ];
	};

	using ShaderResource = core::graphics::direct_3d_11::ShaderResourceWithData< ShaderResourceData >;

	static const int shader_resource_view_slot_ = 1;		/// !!!!!!!!!!!

private:
	Direct3D11*								direct_3d_;
	std::unique_ptr< ShaderResource >		shader_resource_;
	std::unique_ptr< Direct3D11Texture >	texture_;

	int							cascade_levels_;			///< CSM のレベル数

	ID3D11Texture2D*			depth_stencil_texture_;
	ID3D11DepthStencilView*		depth_stencil_view_;

	ViewportList				viewport_list_;

	/// @todo static をやめる
	static Vector				light_position_;			///< ( 0, 0, 0 ) を向く sun

	static Matrix				view_matrix_;
	static MatrixList			projection_matrix_list_;

protected:
	const Matrix get_view_projection_matrix( uint_t level ) const { return view_matrix_ * projection_matrix_list_[ level ]; }

public:
	ShadowMap( Direct3D11*, uint_t, size_t );
	virtual ~ShadowMap();

	void ready_to_render_shadow_map() override;
	void ready_to_render_shadow_map_with_cascade_level( uint_t ) override;

	void ready_to_render_scene() override;
	void bind_to_render_scene() const override;

	int get_cascade_levels() const override { return cascade_levels_; }
	
	void set_light_position( const Vector& ) override;
	void set_eye_position( const Vector& ) override;

	const ShaderResource* get_shader_resource() const override { return shader_resource_.get(); }

	Texture* get_texture() override { return texture_.get(); }
	const Texture* get_texture() const override { return texture_.get(); }
	
}; // class ShadowMap

} // namespace core::graphics::direct_3d_11
