#include "pch.h"

#include <core/graphics/Frustum.h>

#include <blue_sky/ConstantBuffers.h>
#include <blue_sky/graphics/Model.h>
#include <blue_sky/ecs/system/ObjectRenderDataSystem.h>
#include <blue_sky/ecs/component/ObjectRenderComponent.h>

#include <core/ecs/EntityManager.h>
#include <core/ecs/component/TransformComponent.h>

#include "FakeGraphics.h"

#include <filesystem>
#include <fstream>
#include <string>

/**
 * 見えない物を描かない（カリング）のテスト : 視錐台と球の判定・線の頂点を囲む箱・モデルを囲む球・描く対象の振り分け
 */

using core::graphics::Frustum;
using core::Matrix;
using core::Vector;
using blue_sky::ecs::ObjectRenderDataSystem;
using blue_sky::ecs::ObjectRenderComponent;
using test_graphics::FakeConstantBuffer;
using test_graphics::FakeLine;
using test_graphics::FakeResourceLoader;

namespace
{

constexpr float Pi = 3.14159265f;

/// 原点から +z を見るカメラ（縦の視野 90 度・横長 1:1・near 1・far 100）
Matrix make_camera()
{
	const Matrix view = Matrix().set_look_at( Vector( 0.f, 0.f, 0.f, 1.f ), Vector( 0.f, 0.f, 1.f, 1.f ), Vector( 0.f, 1.f, 0.f, 0.f ) );
	const Matrix projection = Matrix().set_perspective_fov( Pi / 2.f, 1.f, 1.f, 100.f );

	return view * projection;
}

/// 一時的な線の OBJ ファイル（壊れるときに消す）
struct TemporaryLineObj
{
	std::filesystem::path path;

	explicit TemporaryLineObj( const std::string& name, const std::string& text )
		: path( std::filesystem::temp_directory_path() / name )
	{
		std::ofstream out( path );
		out << text;
	}

	~TemporaryLineObj()
	{
		std::error_code e;
		std::filesystem::remove( path, e );
	}
};

/// 線だけのモデル（-0.5〜0.5 の立方体の 3 辺。いちばん長い線分は 1）
blue_sky::graphics::Model* add_line_model( FakeResourceLoader& resources, const char* name )
{
	static TemporaryLineObj obj( "blue-sky-culling-test-line.obj",
		"v -0.5 -0.5 -0.5 0 0 0 1\n"
		"v  0.5 -0.5 -0.5 0 0 0 1\n"
		"v  0.5  0.5 -0.5 0 0 0 1\n"
		"v  0.5  0.5  0.5 0 0 0 1\n"
		"f 1 2\n"
		"f 2 3\n"
		"f 3 4\n" );

	auto* model = resources.add_model( name );
	auto* line = new FakeLine();
	EXPECT_TRUE( line->load_obj( obj.path.string().c_str() ) );
	model->set_line( line );

	return model;
}

} // namespace

// ---- 視錐台と球 ----

TEST( FrustumTest, 透視のカメラの前の球は中_後ろ_横_奥の外_手前の外は外 )
{
	const Frustum f = Frustum::from_view_projection( make_camera() );

	EXPECT_TRUE( f.intersects_sphere( Vector( 0.f, 0.f, 10.f ), 0.5f ) );		// 前
	EXPECT_FALSE( f.intersects_sphere( Vector( 0.f, 0.f, -10.f ), 0.5f ) );		// 後ろ
	EXPECT_FALSE( f.intersects_sphere( Vector( 20.f, 0.f, 10.f ), 0.5f ) );		// 右の外（視野 90 度なので z = 10 の右の端は x = 10）
	EXPECT_FALSE( f.intersects_sphere( Vector( -20.f, 0.f, 10.f ), 0.5f ) );	// 左の外
	EXPECT_FALSE( f.intersects_sphere( Vector( 0.f, 20.f, 10.f ), 0.5f ) );		// 上の外
	EXPECT_FALSE( f.intersects_sphere( Vector( 0.f, -20.f, 10.f ), 0.5f ) );	// 下の外
	EXPECT_FALSE( f.intersects_sphere( Vector( 0.f, 0.f, 110.f ), 0.5f ) );		// 奥（far 100）の外
	EXPECT_FALSE( f.intersects_sphere( Vector( 0.f, 0.f, 0.2f ), 0.5f ) );		// 手前（near 1）の外
}

TEST( FrustumTest, 面に掛かる球は中_面から半径より離れると外 )
{
	const Frustum f = Frustum::from_view_projection( make_camera() );

	// 右の面は x = z（z = 10 で x = 10）。面からの距離は ( x - z ) / √2
	EXPECT_TRUE( f.intersects_sphere( Vector( 10.5f, 0.f, 10.f ), 0.5f ) );		// 中心は外だが、球が面に掛かる（距離 0.35）
	EXPECT_FALSE( f.intersects_sphere( Vector( 11.f, 0.f, 10.f ), 0.5f ) );		// 距離 0.71 > 0.5
	EXPECT_TRUE( f.intersects_sphere( Vector( 11.f, 0.f, 10.f ), 1.f ) );		// 半径を大きくすると掛かる

	// 奥・手前の面
	EXPECT_TRUE( f.intersects_sphere( Vector( 0.f, 0.f, 100.4f ), 0.5f ) );
	EXPECT_TRUE( f.intersects_sphere( Vector( 0.f, 0.f, 0.6f ), 0.5f ) );
}

TEST( FrustumTest, 平行の投影_影のカスケード_は幅と深さの箱 )
{
	// 上（y = 50）から下を見る、幅 10 の平行の投影（near 1・far 100）
	const Matrix view = Matrix().set_look_at( Vector( 0.f, 50.f, 0.f, 1.f ), Vector( 0.f, 0.f, 0.f, 1.f ), Vector( 0.f, 0.f, 1.f, 0.f ) );
	const Matrix projection = Matrix().set_orthographic( 10.f, 10.f, 1.f, 100.f );
	const Frustum f = Frustum::from_view_projection( view * projection );

	EXPECT_TRUE( f.intersects_sphere( Vector( 0.f, 0.f, 0.f ), 0.1f ) );
	EXPECT_TRUE( f.intersects_sphere( Vector( 4.9f, 0.f, -4.9f ), 0.1f ) );
	EXPECT_FALSE( f.intersects_sphere( Vector( 6.f, 0.f, 0.f ), 0.5f ) );		// 横の外（幅 10 なので ±5）
	EXPECT_TRUE( f.intersects_sphere( Vector( 5.4f, 0.f, 0.f ), 0.5f ) );		// 球が端に掛かる
	EXPECT_FALSE( f.intersects_sphere( Vector( 0.f, 0.f, 6.f ), 0.5f ) );
	EXPECT_FALSE( f.intersects_sphere( Vector( 0.f, -60.f, 0.f ), 0.5f ) );		// 奥（光から 110）の外
	EXPECT_FALSE( f.intersects_sphere( Vector( 0.f, 49.5f, 0.f ), 0.2f ) );		// 手前（光から 1 未満）の外
}

TEST( FrustumTest, 既定の視錐台はどの球も外さない )
{
	const Frustum f;

	EXPECT_TRUE( f.intersects_sphere( Vector( 1e6f, -1e6f, 1e6f ), 0.f ) );
}

// ---- 囲む箱・囲む球 ----

TEST( CullingTest, 線を読むと頂点を囲む箱といちばん長い線分を覚える_z_は反転して読む )
{
	TemporaryLineObj obj( "blue-sky-culling-test-line2.obj",
		"v 0 0 1 0 0 0 1\n"
		"v 3 0 1 0 0 0 1\n"
		"v 3 4 2 0 0 0 1\n"
		"f 1 2\n"
		"f 2 3\n" );

	FakeLine line;
	ASSERT_TRUE( line.load_obj( obj.path.string().c_str() ) );

	const auto& b = line.get_bounds();
	ASSERT_FALSE( b.empty );
	EXPECT_FLOAT_EQ( b.min.x(), 0.f );
	EXPECT_FLOAT_EQ( b.max.x(), 3.f );
	EXPECT_FLOAT_EQ( b.max.y(), 4.f );
	EXPECT_FLOAT_EQ( b.min.z(), -2.f );		// z は反転して読む（以前と同じ）
	EXPECT_FLOAT_EQ( b.max.z(), -1.f );

	EXPECT_FLOAT_EQ( line.get_max_segment_length(), std::sqrt( 17.f ) );		// ( 3, 0, -1 ) → ( 3, 4, -2 )
}

TEST( CullingTest, モデルを囲む球は箱を囲む球に余裕を足したもの_何も無ければ求めない )
{
	FakeResourceLoader resources;
	auto* model = add_line_model( resources, "line" );

	core::Vector3 center;
	float_t radius = 0.f;
	ASSERT_TRUE( model->get_cull_sphere( center, radius ) );

	// 箱は ( -0.5, -0.5, -0.5 )〜( 0.5, 0.5, 0.5 )（z を反転しても同じ）。半径 √3/2 に 1 割と 0.05、線分の長さ 1 の 2 倍を足す
	EXPECT_NEAR( center.x(), 0.f, 1e-6f );
	EXPECT_NEAR( center.z(), 0.f, 1e-6f );
	EXPECT_NEAR( radius, std::sqrt( 3.f ) / 2.f * 1.1f + 0.05f + 2.f, 1e-5f );

	// メッシュも線も無いモデルはカリングしない
	auto* empty = resources.add_model( "empty" );
	EXPECT_FALSE( empty->get_cull_sphere( center, radius ) );
}

// ---- 描く対象の振り分け（ObjectRenderDataSystem） ----

namespace
{

struct CullingFixture
{
	FakeResourceLoader		resources;
	core::ecs::EntityManager	manager;
	ObjectRenderDataSystem*	system = nullptr;
	blue_sky::graphics::Model*	model = nullptr;

	CullingFixture()
	{
		manager.add_system< ObjectRenderDataSystem >( 0, & resources );
		system = manager.get_system< ObjectRenderDataSystem >();
		model = add_line_model( resources, "line" );
	}

	/// 位置 p に描く物を作り、その定数バッファを返す
	FakeConstantBuffer* create( const Vector& p, ObjectRenderComponent** out = nullptr )
	{
		auto* entity = manager.create_entity();
		entity->add_component< core::ecs::TransformComponent >()->transform.set_position( p );

		auto* render = entity->add_component< ObjectRenderComponent >();
		render->model = resources.get_model_handle( model );

		auto buffer = std::make_unique< FakeConstantBuffer >( sizeof( blue_sky::ObjectConstantBufferData ), 0, core::graphics::ConstantBufferUsage::DYNAMIC );
		FakeConstantBuffer* b = buffer.get();
		render->object_constant_buffer = std::move( buffer );

		if ( out )
		{
			* out = render;
		}

		return b;
	}

	const ObjectRenderDataSystem::RenderTarget& target( size_t n ) const { return system->get_render_targets()[ n ]; }
};

} // namespace

TEST( CullingTest, 範囲の外の物は描く段から外れ_どこにも入らない物は定数を書かない )
{
	CullingFixture f;

	FakeConstantBuffer* front = f.create( Vector( 0.f, 0.f, 10.f ) );		// カメラの前
	FakeConstantBuffer* behind = f.create( Vector( 0.f, 0.f, -10.f ) );		// カメラの後ろで、影の範囲にも入らない
	FakeConstantBuffer* shadow_only = f.create( Vector( 50.f, 0.f, -30.f ) );	// カメラの後ろだが、影の段 1 の範囲に入る

	// 影の段 0 は原点の近く（幅 10）、段 1 は ( 50, *, -30 ) の近く（幅 10）。どちらも上から見る平行の投影
	auto make_shadow = [] ( float x, float z )
	{
		const Matrix view = Matrix().set_look_at( Vector( x, 50.f, z, 1.f ), Vector( x, 0.f, z, 1.f ), Vector( 0.f, 0.f, 1.f, 0.f ) );
		return Frustum::from_view_projection( view * Matrix().set_orthographic( 10.f, 10.f, 1.f, 100.f ) );
	};

	const Frustum shadows[ 2 ] = { make_shadow( 0.f, 10.f ), make_shadow( 50.f, -30.f ) };

	f.system->set_culling( Frustum::from_view_projection( make_camera() ), shadows, 2 );
	f.system->update_render_data();

	// カメラの前の物 : 本体の段と影の段 0 に入る
	EXPECT_TRUE( f.system->is_visible( f.target( 0 ), ObjectRenderDataSystem::VisibleInCamera ) );
	EXPECT_TRUE( f.system->is_visible( f.target( 0 ), ObjectRenderDataSystem::visible_in_shadow_cascade( 0 ) ) );
	EXPECT_FALSE( f.system->is_visible( f.target( 0 ), ObjectRenderDataSystem::visible_in_shadow_cascade( 1 ) ) );
	EXPECT_EQ( front->update_count, 1 );

	// どこにも入らない物 : どの段にも入らず、定数を書かない
	EXPECT_FALSE( f.system->is_visible( f.target( 1 ), ObjectRenderDataSystem::VisibleInCamera ) );
	EXPECT_FALSE( f.system->is_visible( f.target( 1 ), ObjectRenderDataSystem::visible_in_shadow_cascade( 0 ) ) );
	EXPECT_FALSE( f.system->is_visible( f.target( 1 ), ObjectRenderDataSystem::visible_in_shadow_cascade( 1 ) ) );
	EXPECT_EQ( behind->update_count, 0 );

	// 影の段だけに入る物 : 本体は描かないが、影を描くので定数は書く
	EXPECT_FALSE( f.system->is_visible( f.target( 2 ), ObjectRenderDataSystem::VisibleInCamera ) );
	EXPECT_TRUE( f.system->is_visible( f.target( 2 ), ObjectRenderDataSystem::visible_in_shadow_cascade( 1 ) ) );
	EXPECT_EQ( shadow_only->update_count, 1 );

	const auto& stats = f.system->get_culling_stats();
	EXPECT_EQ( stats.targets, 3u );
	EXPECT_EQ( stats.always_visible, 0u );
	EXPECT_EQ( stats.in_camera, 1u );
	EXPECT_EQ( stats.in_shadow_cascade[ 0 ], 1u );
	EXPECT_EQ( stats.in_shadow_cascade[ 1 ], 1u );

	// 次のフレームで範囲を渡さなければ、全部を描き、全部の定数を書く（以前と同じ）
	f.system->forget_resolved_models();
	f.system->update_render_data();

	EXPECT_TRUE( f.system->is_visible( f.target( 1 ), ObjectRenderDataSystem::VisibleInCamera ) );
	EXPECT_EQ( behind->update_count, 1 );
	EXPECT_EQ( front->update_count, 2 );
}

TEST( CullingTest, 範囲から外れて定数を書かない間も前フレームの_world_は進み_戻ると今の_world_を書く )
{
	CullingFixture f;

	ObjectRenderComponent* render = nullptr;
	FakeConstantBuffer* buffer = f.create( Vector( 0.f, 0.f, 10.f ), & render );
	auto* transform = f.manager.find( f.target( 0 ).id )->get_component< core::ecs::TransformComponent >();

	const Frustum camera = Frustum::from_view_projection( make_camera() );

	f.system->set_culling( camera, nullptr, 0 );
	f.system->update_render_data();
	f.system->forget_resolved_models();
	ASSERT_EQ( buffer->update_count, 1 );

	// 後ろへ動かす : 書かないが、前フレームの world は今の位置になる
	transform->transform.set_position( Vector( 0.f, 0.f, -10.f ) );
	f.system->set_culling( camera, nullptr, 0 );
	f.system->update_render_data();
	f.system->forget_resolved_models();
	EXPECT_EQ( buffer->update_count, 1 );
	EXPECT_FLOAT_EQ( ( Vector( 0.f, 0.f, 0.f, 1.f ) * render->prev_world ).z(), -10.f );

	// 前へ戻す : 今の world を書き、前フレームの world は後ろの位置（以前と同じ。毎フレーム書いていたときと同じ値）
	transform->transform.set_position( Vector( 0.f, 0.f, 20.f ) );
	f.system->set_culling( camera, nullptr, 0 );
	f.system->update_render_data();
	EXPECT_EQ( buffer->update_count, 2 );

	const auto& data = buffer->as< blue_sky::ObjectConstantBufferData >();
	EXPECT_FLOAT_EQ( ( Vector( 0.f, 0.f, 0.f, 1.f ) * data.world ).z(), 20.f );
	EXPECT_FLOAT_EQ( ( Vector( 0.f, 0.f, 0.f, 1.f ) * data.prev_world ).z(), -10.f );
}
