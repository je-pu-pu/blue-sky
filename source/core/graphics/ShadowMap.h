#pragma once

#include <type/type.h>

namespace core::graphics
{
	class Texture;
	class ConstantBuffer;

/**
 * 抽象シャドウマップ
 *
 */
class ShadowMap
{
public:
	static const int MaxCascadeLevels = 4;

	using Vector	= Vector;
	using Texture	= Texture;

	/// 既定の値（以前 Direct3D11 の ShadowMap::set_light_position に直接書かれていた値）
	static constexpr float_t default_near_clip = 1.f;
	static constexpr float_t default_far_clip = 500.f;
	static constexpr float_t default_cascade_width[ MaxCascadeLevels ] = { 10.f, 50.f, 150.f, 9999.f };

	/// カスケードごとの、シーンの描画でそのカスケードを使う深さの範囲（幅に掛ける割合）
	static constexpr float_t view_depth_per_cascade_width = 0.3f;

private:
	float_t near_clip_ = default_near_clip;		///< 光から見た投影の near clip
	float_t far_clip_ = default_far_clip;		///< 光から見た投影の far clip（地面が範囲外になるときは大きくする）
	float_t cascade_width_[ MaxCascadeLevels ] = { default_cascade_width[ 0 ], default_cascade_width[ 1 ], default_cascade_width[ 2 ], default_cascade_width[ 3 ] };	///< カスケードごとの、光から見た投影の幅

protected:
	/// 投影の設定（near / far clip・カスケードの幅）が変わったときに呼ばれる。実装は投影行列を作り直す
	virtual void on_projection_changed() { }

public:
	ShadowMap() { }
	virtual ~ShadowMap() { }

	float_t get_near_clip() const { return near_clip_; }
	float_t get_far_clip() const { return far_clip_; }

	/**
	 * 光から見た投影の near / far clip を設定する
	 *
	 * @return 設定したら true。0 < near < far でなければ false（前の値のまま）
	 */
	bool set_clip( float_t near_clip, float_t far_clip )
	{
		if ( ! ( near_clip > 0.f && far_clip > near_clip ) )
		{
			return false;
		}

		near_clip_ = near_clip;
		far_clip_ = far_clip;

		on_projection_changed();

		return true;
	}

	/// far clip だけを設定する（near clip は今のまま）
	bool set_far_clip( float_t far_clip ) { return set_clip( near_clip_, far_clip ); }

	/// カスケードの幅（範囲外のレベルは 0）
	float_t get_cascade_width( uint_t level ) const { return level < MaxCascadeLevels ? cascade_width_[ level ] : 0.f; }

	/**
	 * カスケードの幅を設定する
	 *
	 * @return 設定したら true。レベルが範囲外か、幅が 0 以下なら false（前の値のまま）
	 */
	bool set_cascade_width( uint_t level, float_t width )
	{
		if ( level >= MaxCascadeLevels || ! ( width > 0.f ) )
		{
			return false;
		}

		cascade_width_[ level ] = width;

		on_projection_changed();

		return true;
	}

	/// シーンの描画でカスケード level を使う深さの範囲
	float_t get_view_depth_for_cascade( uint_t level ) const { return get_cascade_width( level ) * view_depth_per_cascade_width; }

	virtual void ready_to_render_shadow_map() = 0;
	virtual void ready_to_render_shadow_map_with_cascade_level( uint_t ) = 0;

	virtual void ready_to_render_scene() = 0;
	virtual void bind_to_render_scene() const = 0;

	virtual int get_cascade_levels() const = 0;
	
	virtual void set_light_position( const Vector& ) = 0;
	virtual void set_eye_position( const Vector& ) = 0;

	virtual const ConstantBuffer* get_constant_buffer() const = 0;

	virtual Texture* get_texture() = 0;
	virtual const Texture* get_texture() const = 0;

}; // class ShadowMap

} // core::graphics
