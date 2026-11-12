#pragma once

#include "Texture.h"
#include <core/graphics/ConstantBufferWithData.h>
#include <core/graphics/ShadowMap.h>
#include <d3d11.h>
#include <vector>

namespace core::graphics::direct_3d_11
{
	class Direct3D11;

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

	struct ConstantBufferData
	{
		Matrix shadow_view_projection[ 3 ];
		float_t view_depth_per_cascade_level[ 4 ] = { 0.f };
	};

	using ConstantBuffer = core::graphics::ConstantBufferWithData< ConstantBufferData, 10 >;

	static const int shader_resource_view_slot_ = 1;		/// !!!!!!!!!!!

private:
	Direct3D11*								direct_3d_;
	std::unique_ptr< ConstantBuffer >		shader_resource_;
	std::unique_ptr< Texture >				texture_;

	int							cascade_levels_;			///< CSM のレベル数

	com_ptr< ID3D11Texture2D >			depth_stencil_texture_;
	com_ptr< ID3D11DepthStencilView >	depth_stencil_view_;

	ViewportList				viewport_list_;

	// 以前は static で、全インスタンス（作り直したゲームのものも含む）が光の位置と行列を共有していた
	Vector						light_position_;			///< ( 0, 0, 0 ) を向く sun

	Matrix						view_matrix_;
	MatrixList					projection_matrix_list_;

	void update_projection();

protected:
	const Matrix get_view_projection_matrix( uint_t level ) const { return view_matrix_ * projection_matrix_list_[ level ]; }

	void on_projection_changed() override { update_projection(); }

public:
	ShadowMap( Direct3D11*, uint_t, size_t );
	virtual ~ShadowMap();

	void ready_to_render_shadow_map() override;
	void ready_to_render_shadow_map_with_cascade_level( uint_t ) override;

	void ready_to_render_scene() override;
	void bind_to_render_scene() const override;

	int get_cascade_levels() const override { return cascade_levels_; }

	/// ready_to_render_shadow_map_with_cascade_level がシェーダーに渡すのと同じ行列（光の位置・カメラの位置・カスケードの幅から）
	bool get_cascade_view_projection( uint_t level, Matrix& view_projection ) const override
	{
		if ( level >= static_cast< uint_t >( cascade_levels_ ) )
		{
			return false;
		}

		view_projection = get_view_projection_matrix( level );
		return true;
	}

	void set_light_position( const Vector& ) override;
	void set_eye_position( const Vector& ) override;

	const ConstantBuffer* get_constant_buffer() const override { return shader_resource_.get(); }

	Texture* get_texture() override { return texture_.get(); }
	const Texture* get_texture() const override { return texture_.get(); }
	
}; // class ShadowMap

} // namespace core::graphics::direct_3d_11
