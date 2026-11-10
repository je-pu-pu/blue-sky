#include "pch.h"

// 描画の設定（光の向き・定数バッファの更新方式・シャドウマップの範囲・描画先テクスチャの指定）のうち、D3D11 に依存しない部分のテスト
#include <core/graphics/LightDirection.h>
#include <core/graphics/ConstantBuffer.h>
#include <core/graphics/ShadowMap.h>
#include <core/graphics/RenderTargetTextureOptions.h>

#include <cmath>

namespace {

using core::graphics::LightDirection;
using core::graphics::ConstantBuffer;
using core::graphics::ConstantBufferUsage;
using core::graphics::RenderTargetTextureOptions;
using core::Vector;

// ---- 光の向き ----

TEST( LightDirectionTest, 既定は以前の固定値と同じ )
{
	LightDirection light;

	EXPECT_FLOAT_EQ( light.get().x(), -1.f );
	EXPECT_FLOAT_EQ( light.get().y(), -2.f );
	EXPECT_FLOAT_EQ( light.get().z(), 0.5f );

	// 以前の setup_rendering と同じ計算で同じ値になる
	const Vector expected = Vector( -1.f, -2.f, 0.5f, 0.f ).normalize();
	const Vector actual = light.get_normalized();

	EXPECT_FLOAT_EQ( actual.x(), expected.x() );
	EXPECT_FLOAT_EQ( actual.y(), expected.y() );
	EXPECT_FLOAT_EQ( actual.z(), expected.z() );
	EXPECT_FLOAT_EQ( actual.w(), 0.f );
}

TEST( LightDirectionTest, 設定した向きを正規化して返す )
{
	LightDirection light;

	EXPECT_TRUE( light.set( Vector( 0.f, -10.f, 0.f, 1.f ) ) );

	EXPECT_FLOAT_EQ( light.get().y(), -10.f );
	EXPECT_FLOAT_EQ( light.get().w(), 0.f );		// w は 0 にする（向きなので）

	const Vector n = light.get_normalized();

	EXPECT_FLOAT_EQ( n.x(), 0.f );
	EXPECT_FLOAT_EQ( n.y(), -1.f );
	EXPECT_FLOAT_EQ( n.z(), 0.f );
}

TEST( LightDirectionTest, 長さ0の向きは受け付けず前の向きのまま )
{
	LightDirection light;
	light.set( Vector( 1.f, 0.f, 0.f ) );

	EXPECT_FALSE( light.set( Vector( 0.f, 0.f, 0.f ) ) );
	EXPECT_FLOAT_EQ( light.get().x(), 1.f );
	EXPECT_FALSE( std::isnan( light.get_normalized().x() ) );
}

TEST( LightDirectionTest, resetで既定に戻る )
{
	LightDirection light;
	light.set( Vector( 1.f, 0.f, 0.f ) );
	light.reset();

	EXPECT_FLOAT_EQ( light.get().x(), -1.f );
	EXPECT_FLOAT_EQ( light.get().y(), -2.f );
}

// ---- 定数バッファの更新方式 ----

class FakeConstantBuffer : public ConstantBuffer
{
public:
	using ConstantBuffer::ConstantBuffer;

	void update( const void* ) const override { }
	void bind_to_vs() const override { }
	void bind_to_hs() const override { }
	void bind_to_ds() const override { }
	void bind_to_cs() const override { }
	void bind_to_gs() const override { }
	void bind_to_ps() const override { }
};

TEST( ConstantBufferUsageTest, 既定はDEFAULTで以前と同じ )
{
	FakeConstantBuffer b;
	EXPECT_EQ( b.get_usage(), ConstantBufferUsage::DEFAULT );
}

TEST( ConstantBufferUsageTest, 作るときにDYNAMICを選べる )
{
	FakeConstantBuffer b( ConstantBufferUsage::DYNAMIC );
	EXPECT_EQ( b.get_usage(), ConstantBufferUsage::DYNAMIC );
}

TEST( ConstantBufferUsageTest, 大きさは16バイトの倍数に切り上げる )
{
	EXPECT_EQ( ConstantBuffer::get_aligned_size( 0 ), 0u );
	EXPECT_EQ( ConstantBuffer::get_aligned_size( 1 ), 16u );
	EXPECT_EQ( ConstantBuffer::get_aligned_size( 16 ), 16u );
	EXPECT_EQ( ConstantBuffer::get_aligned_size( 17 ), 32u );
	EXPECT_EQ( ConstantBuffer::get_aligned_size( 144 ), 144u );
}

// ---- シャドウマップの範囲 ----

class FakeShadowMap : public core::graphics::ShadowMap
{
public:
	int projection_changed_count = 0;

	void ready_to_render_shadow_map() override { }
	void ready_to_render_shadow_map_with_cascade_level( uint_t ) override { }
	void ready_to_render_scene() override { }
	void bind_to_render_scene() const override { }
	int get_cascade_levels() const override { return 3; }
	void set_light_position( const Vector& ) override { }
	void set_eye_position( const Vector& ) override { }
	const core::graphics::ConstantBuffer* get_constant_buffer() const override { return nullptr; }
	Texture* get_texture() override { return nullptr; }
	const Texture* get_texture() const override { return nullptr; }

protected:
	void on_projection_changed() override { projection_changed_count++; }
};

TEST( ShadowMapSettingsTest, 既定は以前の固定値と同じ )
{
	FakeShadowMap s;

	EXPECT_FLOAT_EQ( s.get_near_clip(), 1.f );
	EXPECT_FLOAT_EQ( s.get_far_clip(), 500.f );
	EXPECT_FLOAT_EQ( s.get_cascade_width( 0 ), 10.f );
	EXPECT_FLOAT_EQ( s.get_cascade_width( 1 ), 50.f );
	EXPECT_FLOAT_EQ( s.get_cascade_width( 2 ), 150.f );
	EXPECT_FLOAT_EQ( s.get_cascade_width( 3 ), 9999.f );
	EXPECT_FLOAT_EQ( s.get_view_depth_for_cascade( 1 ), 50.f * 0.3f );
}

TEST( ShadowMapSettingsTest, far_clipを変えると投影を作り直す )
{
	FakeShadowMap s;

	EXPECT_TRUE( s.set_far_clip( 1000.f ) );
	EXPECT_FLOAT_EQ( s.get_far_clip(), 1000.f );
	EXPECT_FLOAT_EQ( s.get_near_clip(), 1.f );
	EXPECT_EQ( s.projection_changed_count, 1 );
}

TEST( ShadowMapSettingsTest, nearとfarの順がおかしい値は受け付けない )
{
	FakeShadowMap s;

	EXPECT_FALSE( s.set_far_clip( 0.5f ) );			// near より手前
	EXPECT_FALSE( s.set_clip( 0.f, 100.f ) );		// near が 0
	EXPECT_FALSE( s.set_clip( 10.f, 10.f ) );		// 同じ
	EXPECT_FLOAT_EQ( s.get_far_clip(), 500.f );
	EXPECT_EQ( s.projection_changed_count, 0 );
}

TEST( ShadowMapSettingsTest, カスケードの幅を変えられる )
{
	FakeShadowMap s;

	EXPECT_TRUE( s.set_cascade_width( 2, 300.f ) );
	EXPECT_FLOAT_EQ( s.get_cascade_width( 2 ), 300.f );
	EXPECT_FLOAT_EQ( s.get_view_depth_for_cascade( 2 ), 300.f * 0.3f );
	EXPECT_EQ( s.projection_changed_count, 1 );

	EXPECT_FALSE( s.set_cascade_width( 4, 10.f ) );		// 範囲外のレベル
	EXPECT_FALSE( s.set_cascade_width( 0, 0.f ) );		// 幅 0
	EXPECT_FLOAT_EQ( s.get_cascade_width( 0 ), 10.f );
	EXPECT_FLOAT_EQ( s.get_cascade_width( 4 ), 0.f );
	EXPECT_EQ( s.projection_changed_count, 1 );
}

// ---- 描画先テクスチャの指定 ----

TEST( RenderTargetTextureOptionsTest, 既定はミップ1段で画面と同じMSAA )
{
	RenderTargetTextureOptions o;

	EXPECT_EQ( o.mip_levels, 1u );
	EXPECT_EQ( o.sample_count, RenderTargetTextureOptions::SAMPLE_COUNT_SAME_AS_SCREEN );

	const auto n = o.normalized( 4 );

	EXPECT_EQ( n.mip_levels, 1u );
	EXPECT_EQ( n.sample_count, RenderTargetTextureOptions::SAMPLE_COUNT_SAME_AS_SCREEN );
}

TEST( RenderTargetTextureOptionsTest, MSAAなしならミップを持てる )
{
	RenderTargetTextureOptions o;
	o.mip_levels = 5;
	o.sample_count = 1;
	o.sample_quality = 3;

	const auto n = o.normalized( 4 );

	EXPECT_EQ( n.mip_levels, 5u );
	EXPECT_EQ( n.sample_count, 1u );
	EXPECT_EQ( n.sample_quality, 0u );		// MSAA なしの品質は 0
}

TEST( RenderTargetTextureOptionsTest, MSAAならミップは1段にする )
{
	RenderTargetTextureOptions o;
	o.mip_levels = 5;
	o.sample_count = 4;

	EXPECT_EQ( o.normalized( 1 ).mip_levels, 1u );

	// 画面と同じ MSAA で、画面が MSAA ならミップは 1 段。画面が MSAA なしならミップを持てる
	RenderTargetTextureOptions same;
	same.mip_levels = 3;

	EXPECT_EQ( same.normalized( 4 ).mip_levels, 1u );
	EXPECT_EQ( same.normalized( 1 ).mip_levels, 3u );
}

TEST( RenderTargetTextureOptionsTest, ミップ0段は1段にする )
{
	RenderTargetTextureOptions o;
	o.mip_levels = 0;
	o.sample_count = 1;

	EXPECT_EQ( o.normalized( 1 ).mip_levels, 1u );
}

} // namespace
