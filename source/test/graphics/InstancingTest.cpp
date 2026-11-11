#include "pch.h"

#include <core/graphics/Frustum.h>
#include <core/animation/SkinningAnimationSet.h>

#include <blue_sky/ConstantBuffers.h>
#include <blue_sky/graphics/Model.h>
#include <blue_sky/graphics/Mesh.h>
#include <blue_sky/graphics/shader/BaseShader.h>
#include <blue_sky/ecs/system/ObjectRenderDataSystem.h>
#include <blue_sky/ecs/component/ObjectRenderComponent.h>

#include <core/ecs/EntityManager.h>
#include <core/ecs/component/TransformComponent.h>

#include "FakeGraphics.h"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

/**
 * 同じモデルの物体をまとめて描く（インスタンス描画）のテスト : 描く段ごとの描く物の並び・まとめる条件・物体ごとの定数
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

/// メッシュの頂点バッファの偽物（何もしない）
class FakeMeshBuffer : public blue_sky::graphics::Mesh::Buffer
{
public:
	FakeMeshBuffer() : Buffer( Type::DEFAULT ) { }

	void create_vertex_buffer() override { }
	void create_index_buffer() override { }
	void bind() const override { }
	void render( uint_t ) const override { }
	void render_instanced( uint_t, uint_t ) const override { }
};

/// シェーダーの偽物（まとめて描けるかだけを決める）
class FakeShader : public blue_sky::graphics::shader::BaseShader
{
private:
	bool can_instance_;

public:
	explicit FakeShader( bool can_instance ) : BaseShader( nullptr ), can_instance_( can_instance ) { }

	FakeShader* clone() const override { return new FakeShader( *this ); }
	void reload() override { }

	Texture* get_texture_at( uint_t ) override { return nullptr; }
	const Texture* get_texture_at( uint_t ) const override { return nullptr; }
	void set_texture_at( uint_t, Texture* ) override { }

	void bind() const override { }
	void render( const Mesh*, uint_t ) const override { }

	bool can_render_object_instances() const override { return can_instance_; }
};

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

/// 原点から +z を見るカメラ（縦の視野 90 度・near 1・far 100）
Matrix make_camera()
{
	const Matrix view = Matrix().set_look_at( Vector( 0.f, 0.f, 0.f, 1.f ), Vector( 0.f, 0.f, 1.f, 1.f ), Vector( 0.f, 1.f, 0.f, 0.f ) );
	const Matrix projection = Matrix().set_perspective_fov( 3.14159265f / 2.f, 1.f, 1.f, 100.f );

	return view * projection;
}

/// ( x, *, z ) の上から見る、幅 10 の影の段の範囲
Frustum make_shadow( float x, float z )
{
	const Matrix view = Matrix().set_look_at( Vector( x, 50.f, z, 1.f ), Vector( x, 0.f, z, 1.f ), Vector( 0.f, 0.f, 1.f, 0.f ) );
	return Frustum::from_view_projection( view * Matrix().set_orthographic( 10.f, 10.f, 1.f, 100.f ) );
}

struct InstancingFixture
{
	FakeResourceLoader							resources;
	core::ecs::EntityManager					manager;
	ObjectRenderDataSystem*						system = nullptr;

	std::vector< std::unique_ptr< blue_sky::graphics::Mesh > >	meshes;
	std::vector< std::unique_ptr< FakeShader > >				shaders;
	core::SkinningAnimationSet									skinning;

	InstancingFixture()
	{
		manager.add_system< ObjectRenderDataSystem >( 0, & resources );
		system = manager.get_system< ObjectRenderDataSystem >();
		system->set_instancing_enabled( true );
	}

	/// 原点の周り ±0.5 の箱のメッシュを持つモデル（シェーダーは can_instance のもの 1 つ）
	blue_sky::graphics::Model* add_mesh_model( const char* name, bool can_instance = true )
	{
		auto mesh = std::make_unique< blue_sky::graphics::Mesh >( new FakeMeshBuffer() );
		mesh->add_vertex( blue_sky::graphics::Vertex( core::Vector3( -0.5f, -0.5f, -0.5f ) ) );
		mesh->add_vertex( blue_sky::graphics::Vertex( core::Vector3(  0.5f,  0.5f,  0.5f ) ) );
		mesh->calculate_bounds();

		shaders.push_back( std::make_unique< FakeShader >( can_instance ) );

		auto* model = resources.add_model( name );
		model->set_mesh( mesh.get() );
		model->set_shader_at( 0, shaders.back().get() );

		meshes.push_back( std::move( mesh ) );

		return model;
	}

	/// 線を足す（-0.5〜0.5 の立方体の 3 辺）
	void add_line( blue_sky::graphics::Model* model, bool cast_shadow )
	{
		static TemporaryLineObj obj( "blue-sky-instancing-test-line.obj",
			"v -0.5 -0.5 -0.5 1 0 0 1\n"
			"v  0.5 -0.5 -0.5 1 0 0 1\n"
			"v  0.5  0.5 -0.5 1 0 0 1\n"
			"f 1 2\n"
			"f 2 3\n" );

		auto* line = new FakeLine();
		EXPECT_TRUE( line->load_obj( obj.path.string().c_str() ) );
		line->set_cast_shadow( cast_shadow );
		line->set_color( core::Color( 0.25f, 0.5f, 0.75f, 1.f ) );
		model->set_line( line );
	}

	/// 位置 p にモデル model を描く物を作る
	ObjectRenderComponent* create( blue_sky::graphics::Model* model, const Vector& p, FakeConstantBuffer** out_buffer = nullptr )
	{
		auto* entity = manager.create_entity();
		entity->add_component< core::ecs::TransformComponent >()->transform.set_position( p );

		auto* render = entity->add_component< ObjectRenderComponent >();
		render->model = resources.get_model_handle( model );

		auto buffer = std::make_unique< FakeConstantBuffer >( sizeof( blue_sky::ObjectConstantBufferData ), 0, core::graphics::ConstantBufferUsage::DYNAMIC );

		if ( out_buffer )
		{
			* out_buffer = buffer.get();
		}

		render->object_constant_buffer = std::move( buffer );

		return render;
	}

	/// 描く物の並びを「target:まとめの数」（まとめなら "0x3"、1 個ずつなら "1"）で書く
	std::string describe( int pass ) const
	{
		const auto& list = system->get_draw_list( pass );
		std::string s;

		for ( const auto& item : list.items )
		{
			if ( ! s.empty() )
			{
				s += " ";
			}

			s += std::to_string( item.target );

			if ( item.batch != ObjectRenderDataSystem::NoBatch )
			{
				s += "x" + std::to_string( list.batches[ item.batch ].count );
			}
		}

		return s;
	}

	/// まとめ batch に入る物を、描く対象の位置（target）で並べる
	std::vector< std::uint32_t > members( int pass, std::uint32_t batch ) const
	{
		const auto& list = system->get_draw_list( pass );
		const auto& b = list.batches[ batch ];

		std::vector< std::uint32_t > result;

		for ( std::uint32_t n = 0; n < b.count; n++ )
		{
			const std::uint32_t instance = list.instance_indices[ b.first + n ];
			const auto& targets = system->get_render_targets();

			for ( std::uint32_t t = 0; t < targets.size(); t++ )
			{
				if ( targets[ t ].instance == instance )
				{
					result.push_back( t );
				}
			}
		}

		return result;
	}
};

} // namespace

TEST( InstancingTest, 同じモデルの物は最初に出てきた位置でまとめて描き_まとめの中は番号の順 )
{
	InstancingFixture f;
	auto* a = f.add_mesh_model( "a" );
	auto* b = f.add_mesh_model( "b" );

	f.create( a, Vector( 0.f, 0.f, 10.f ) );
	f.create( b, Vector( 1.f, 0.f, 10.f ) );
	f.create( a, Vector( 2.f, 0.f, 10.f ) );
	f.create( a, Vector( 3.f, 0.f, 10.f ) );
	f.create( b, Vector( 4.f, 0.f, 10.f ) );

	f.system->update_render_data();
	ASSERT_TRUE( f.system->is_instancing_ready() );

	EXPECT_EQ( f.describe( ObjectRenderDataSystem::MeshPass ), "0x3 1x2" );
	EXPECT_EQ( f.members( ObjectRenderDataSystem::MeshPass, 0 ), ( std::vector< std::uint32_t >{ 0, 2, 3 } ) );
	EXPECT_EQ( f.members( ObjectRenderDataSystem::MeshPass, 1 ), ( std::vector< std::uint32_t >{ 1, 4 } ) );

	const auto& list = f.system->get_draw_list( ObjectRenderDataSystem::MeshPass );
	EXPECT_EQ( list.batches[ 0 ].model, a );
	EXPECT_EQ( list.batches[ 1 ].model, b );
	EXPECT_EQ( list.batches[ 1 ].first, 3u );		// まとめは番号の表の中で続けて並ぶ
	EXPECT_EQ( list.instance_indices.size(), 5u );
}

TEST( InstancingTest, まとめられない物は今までどおりの位置で1個ずつ描く )
{
	InstancingFixture f;
	auto* a = f.add_mesh_model( "a" );
	auto* no_instancing = f.add_mesh_model( "no_instancing", false );		// シェーダーがまとめて描けない
	auto* skin = f.add_mesh_model( "skin" );
	skin->set_skinning_animation_set( & f.skinning );						// スキンメッシュ

	f.create( no_instancing, Vector( 0.f, 0.f, 10.f ) );
	f.create( a, Vector( 1.f, 0.f, 10.f ) );
	f.create( no_instancing, Vector( 2.f, 0.f, 10.f ) );
	f.create( skin, Vector( 3.f, 0.f, 10.f ) );
	auto* animated = f.create( a, Vector( 4.f, 0.f, 10.f ) );				// ボーンの定数を持つ（アニメーション）
	animated->bone_constant_buffer = std::make_unique< FakeConstantBuffer >( 16, 5, core::graphics::ConstantBufferUsage::DYNAMIC );
	f.create( a, Vector( 5.f, 0.f, 10.f ) );

	f.system->update_render_data();

	EXPECT_EQ( f.describe( ObjectRenderDataSystem::MeshPass ), "0 1x2 2 3 4" );
	EXPECT_EQ( f.members( ObjectRenderDataSystem::MeshPass, 0 ), ( std::vector< std::uint32_t >{ 1, 5 } ) );

	// 影の段 : スキンメッシュでない物はどれもまとめる。スキンメッシュは入らない（別に 1 個ずつ描く）
	EXPECT_EQ( f.describe( ObjectRenderDataSystem::shadow_mesh_pass( 0 ) ), "0x2 1x3" );

	EXPECT_TRUE( a->can_render_object_instances() );
	EXPECT_FALSE( no_instancing->can_render_object_instances() );
	EXPECT_FALSE( skin->can_render_object_instances() );
}

TEST( InstancingTest, 描かない物と範囲の外の物はどの段にも入らず_影の段は段ごとの範囲で決まる )
{
	InstancingFixture f;
	auto* a = f.add_mesh_model( "a" );

	f.create( a, Vector( 0.f, 0.f, 10.f ) );									// カメラの前・影の段 0
	f.create( a, Vector( 0.f, 0.f, -10.f ) );									// どこにも入らない
	f.create( a, Vector( 50.f, 0.f, -30.f ) );									// 影の段 1 だけ
	f.create( a, Vector( 1.f, 0.f, 10.f ) )->is_hidden = true;					// 隠した（風船の点滅）
	f.create( a, Vector( 2.f, 0.f, 10.f ) )->is_mesh_visible = false;			// メッシュを描かない（定数は書く）

	const Frustum shadows[ 2 ] = { make_shadow( 0.f, 10.f ), make_shadow( 50.f, -30.f ) };
	f.system->set_culling( Frustum::from_view_projection( make_camera() ), shadows, 2 );
	f.system->update_render_data();

	EXPECT_EQ( f.describe( ObjectRenderDataSystem::MeshPass ), "0x1" );
	EXPECT_EQ( f.describe( ObjectRenderDataSystem::shadow_mesh_pass( 0 ) ), "0x1" );
	EXPECT_EQ( f.describe( ObjectRenderDataSystem::shadow_mesh_pass( 1 ) ), "2x1" );
	EXPECT_EQ( f.describe( ObjectRenderDataSystem::shadow_mesh_pass( 2 ) ), "" );		// 範囲を渡していない段は、カリングした物を描かない

	// 隠した物は定数を書かない。メッシュを描かない物は定数を書くが、メッシュの段には入らない
	EXPECT_EQ( f.system->get_render_targets()[ 3 ].instance, ObjectRenderDataSystem::NoInstance );
	EXPECT_NE( f.system->get_render_targets()[ 4 ].instance, ObjectRenderDataSystem::NoInstance );
}

TEST( InstancingTest, 線はどれもまとめ_影の段の線は影を落とす線だけ )
{
	InstancingFixture f;
	auto* shadowed = f.add_mesh_model( "shadowed" );
	f.add_line( shadowed, true );
	auto* unshadowed = f.add_mesh_model( "unshadowed", false );
	f.add_line( unshadowed, false );
	auto* skin = f.add_mesh_model( "skin" );
	f.add_line( skin, true );
	skin->set_skinning_animation_set( & f.skinning );

	f.create( shadowed, Vector( 0.f, 0.f, 10.f ) );
	f.create( unshadowed, Vector( 1.f, 0.f, 10.f ) );
	f.create( skin, Vector( 2.f, 0.f, 10.f ) );
	f.create( shadowed, Vector( 3.f, 0.f, 10.f ) )->is_line_visible = false;	// 線を描かない
	f.create( shadowed, Vector( 4.f, 0.f, 10.f ) );

	f.system->update_render_data();

	EXPECT_EQ( f.describe( ObjectRenderDataSystem::LinePass ), "0x2 1x1 2x1" );
	EXPECT_EQ( f.describe( ObjectRenderDataSystem::shadow_line_pass( 0 ) ), "0x2" );
	EXPECT_EQ( f.describe( ObjectRenderDataSystem::MeshPass ), "0x3 1 2" );
}

TEST( InstancingTest, 物体ごとの定数は定数バッファに書いたものと同じ )
{
	InstancingFixture f;
	auto* a = f.add_mesh_model( "a" );
	f.add_line( a, true );

	FakeConstantBuffer* buffer = nullptr;
	f.create( a, Vector( 1.f, 2.f, 10.f ), & buffer );

	f.system->update_render_data();

	const auto& target = f.system->get_render_targets()[ 0 ];
	ASSERT_NE( target.instance, ObjectRenderDataSystem::NoInstance );

	const auto& instance = f.system->get_instances()[ target.instance ];
	EXPECT_EQ( std::memcmp( & instance, buffer->data.data(), sizeof( blue_sky::ObjectConstantBufferData ) ), 0 );
	EXPECT_FLOAT_EQ( instance.color.g(), 0.5f );		// 線の色
	EXPECT_FLOAT_EQ( ( Vector( 0.f, 0.f, 0.f, 1.f ) * instance.world ).y(), 2.f );
}

TEST( InstancingTest, 有効にしていない_描く対象が増えた_フレームが終わったときは準備ができていない )
{
	InstancingFixture f;
	auto* a = f.add_mesh_model( "a" );
	f.create( a, Vector( 0.f, 0.f, 10.f ) );

	// 有効にしていなければ、物体の定数も集めない
	f.system->set_instancing_enabled( false );
	f.system->update_render_data();
	EXPECT_FALSE( f.system->is_instancing_ready() );
	EXPECT_TRUE( f.system->get_instances().empty() );
	f.system->forget_resolved_models();

	f.system->set_instancing_enabled( true );
	f.system->update_render_data();
	EXPECT_TRUE( f.system->is_instancing_ready() );
	EXPECT_EQ( f.system->get_instances().size(), 1u );

	// 描く対象が増えたら、そのフレームはまとめて描かない（並びの位置が変わる）
	f.create( a, Vector( 1.f, 0.f, 10.f ) );
	EXPECT_FALSE( f.system->is_instancing_ready() );

	f.system->update_render_data();
	EXPECT_TRUE( f.system->is_instancing_ready() );
	EXPECT_EQ( f.describe( ObjectRenderDataSystem::MeshPass ), "0x2" );

	// 描く対象が減ったときも同じ
	f.manager.destroy_entity( f.system->get_render_targets()[ 0 ].id );
	EXPECT_FALSE( f.system->is_instancing_ready() );

	// フレームが終わったら、次の update_render_data まで使わない
	f.system->update_render_data();
	EXPECT_TRUE( f.system->is_instancing_ready() );
	EXPECT_EQ( f.describe( ObjectRenderDataSystem::MeshPass ), "0x1" );
	f.system->forget_resolved_models();
	EXPECT_FALSE( f.system->is_instancing_ready() );
}

TEST( InstancingTest, モデルを差し替えて別のモデルになった物は別にまとめる )
{
	InstancingFixture f;
	auto* a = f.add_mesh_model( "a" );

	f.create( a, Vector( 0.f, 0.f, 10.f ) );
	auto* unique = f.create( a, Vector( 1.f, 0.f, 10.f ) );
	f.create( a, Vector( 2.f, 0.f, 10.f ) );

	// make_model_unique と同じ（モデルとシェーダーを複製した、この物だけのモデル）
	unique->model = f.resources.get_model_handle( f.resources.clone_model_with_shaders( a ) );

	f.system->update_render_data();

	EXPECT_EQ( f.describe( ObjectRenderDataSystem::MeshPass ), "0x2 1x1" );
}
