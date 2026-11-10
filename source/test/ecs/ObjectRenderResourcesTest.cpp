#include "pch.h"

#include <blue_sky/ObjectFactory.h>
#include <blue_sky/GameObjectRef.h>
#include <blue_sky/ConstantBuffers.h>
#include <blue_sky/ecs/system/ObjectRenderDataSystem.h>
#include <blue_sky/ecs/component/ObjectRenderComponent.h>
#include <blue_sky/ecs/component/RobotBehaviorComponent.h>

#include <core/ecs/EntityManager.h>
#include <core/ecs/component/TransformComponent.h>
#include <core/physics/PhysicsManager.h>

#include "../graphics/FakeGraphics.h"

/**
 * ゲームの物体の描画の資源（モデル・テクスチャ・定数バッファ）の扱いのテスト
 *
 * 以前は GraphicsManager の全体が要り（純粋仮想を約 70 個埋めた偽物）、テストでは nullptr を渡して描画の所を通らなかった。
 * 今は小さな窓口（ResourceResolver・ResourceLoader）を受け取るので、偽物で中身を確かめる（設計上の問題 D3）
 */

using blue_sky::ObjectFactory;
using blue_sky::GameObjectRef;
using blue_sky::ecs::ObjectRenderDataSystem;
using blue_sky::ecs::ObjectRenderComponent;
using core::ecs::EntityManager;
using core::ecs::TransformComponent;
using test_graphics::FakeResourceLoader;
using test_graphics::FakeConstantBuffer;
using test_graphics::FakeLine;

namespace
{

/// 描く物 1 つ（モデル・定数バッファ付き）を作る
struct RenderFixture
{
	FakeResourceLoader		resources;
	EntityManager			manager;
	ObjectRenderDataSystem*	system = nullptr;

	RenderFixture()
	{
		manager.add_system< ObjectRenderDataSystem >( 0, & resources );
		system = manager.get_system< ObjectRenderDataSystem >();
	}

	/// 線の色を持つモデルで描く物を作り、その定数バッファを返す
	FakeConstantBuffer* create( const char* model_name, ObjectRenderComponent** out_render = nullptr )
	{
		auto* model = resources.add_model( model_name );
		auto* line = new FakeLine();
		line->set_color( core::Color( 0.25f, 0.5f, 0.75f, 1.f ) );
		model->set_line( line );

		auto* entity = manager.create_entity();
		entity->add_component< TransformComponent >()->transform.set_position( core::Vector( 1.f, 2.f, 3.f ) );

		auto* render = entity->add_component< ObjectRenderComponent >();
		render->model = resources.get_model_handle( model );

		auto buffer = std::make_unique< FakeConstantBuffer >( sizeof( blue_sky::ObjectConstantBufferData ), 0, core::graphics::ConstantBufferUsage::DYNAMIC );
		FakeConstantBuffer* p = buffer.get();
		render->object_constant_buffer = std::move( buffer );

		if ( out_render )
		{
			* out_render = render;
		}

		return p;
	}
};

/// 偽の資源で工場を作る（物理の世界は本物）
struct FactoryFixture
{
	FakeResourceLoader				resources;
	core::physics::PhysicsManager	physics;
	EntityManager					manager;
	ObjectFactory					factory{ & manager, & physics, & resources };

	~FactoryFixture()
	{
		// 剛体の持ち主（部品）を、物理の世界より先に消す
		factory.clear();
		manager.clear();
	}

	ObjectRenderComponent* render_of( core::ecs::EntityId id ) { return manager.find( id )->get_component< ObjectRenderComponent >(); }
};

} // namespace

TEST( ObjectRenderResourcesTest, 見える物は_world_と線の色を定数バッファに書く )
{
	RenderFixture f;
	FakeConstantBuffer* buffer = f.create( "box" );

	f.system->update_render_data();

	ASSERT_EQ( buffer->update_count, 1 );

	const auto& data = buffer->as< blue_sky::ObjectConstantBufferData >();
	EXPECT_FLOAT_EQ( data.color.r(), 0.25f );
	EXPECT_FLOAT_EQ( data.color.g(), 0.5f );
	EXPECT_FLOAT_EQ( data.color.b(), 0.75f );

	// world の位置（初回は前フレームの world も同じ）
	const core::Vector p = core::Vector( 0.f, 0.f, 0.f, 1.f ) * data.world;
	EXPECT_FLOAT_EQ( p.x(), 1.f );
	EXPECT_FLOAT_EQ( p.y(), 2.f );
	EXPECT_FLOAT_EQ( p.z(), 3.f );

	const core::Vector q = core::Vector( 0.f, 0.f, 0.f, 1.f ) * data.prev_world;
	EXPECT_FLOAT_EQ( q.x(), 1.f );
}

TEST( ObjectRenderResourcesTest, 見えない物と隠した物の定数は書かない )
{
	RenderFixture f;
	ObjectRenderComponent* invisible = nullptr;
	ObjectRenderComponent* hidden = nullptr;
	FakeConstantBuffer* a = f.create( "a", & invisible );
	FakeConstantBuffer* b = f.create( "b", & hidden );
	FakeConstantBuffer* c = f.create( "c" );

	invisible->is_visible = false;
	hidden->is_hidden = true;

	f.system->update_render_data();

	EXPECT_EQ( a->update_count, 0 );
	EXPECT_EQ( b->update_count, 0 );
	EXPECT_EQ( c->update_count, 1 );

	// 見えない物は前フレームの world も進めない（以前と同じ）
	EXPECT_FALSE( invisible->prev_world_valid );
	EXPECT_FALSE( hidden->prev_world_valid );
}

TEST( ObjectRenderResourcesTest, モデルが解放された物は書かない )
{
	RenderFixture f;
	FakeConstantBuffer* buffer = f.create( "temporary" );

	f.resources.models.remove_named( "temporary" );		// ハンドルは nullptr を返すようになる
	f.resources.models.flush_releases();				// （フレームの終わりの後）
	f.system->update_render_data();

	EXPECT_EQ( buffer->update_count, 0 );
}

TEST( ObjectRenderResourcesTest, 定数バッファの無い物は飛ばす )
{
	RenderFixture f;
	ObjectRenderComponent* render = nullptr;
	f.create( "box", & render );
	render->object_constant_buffer.reset();

	f.system->update_render_data();		// 落ちない

	EXPECT_FALSE( render->prev_world_valid );
}

TEST( ObjectRenderResourcesTest, 工場は種類の名前でモデルを読み毎フレーム書く定数バッファを作る )
{
	FactoryFixture f;

	const auto id = f.factory.create_translation_object( core::Vector( 0.f, 0.f, 0.f ), 1.f, 1.f, 1.f, 1.f );

	const std::vector< std::string > expected = { "create_constant_buffer " + std::to_string( blue_sky::ObjectConstantBuffer::SLOT ), "load_model box-5x5x5" };
	EXPECT_EQ( f.resources.calls, expected );

	ASSERT_EQ( f.resources.buffers.size(), 1u );
	EXPECT_EQ( f.resources.buffers[ 0 ]->size, sizeof( blue_sky::ObjectConstantBufferData ) );
	EXPECT_EQ( f.resources.buffers[ 0 ]->get_usage(), core::graphics::ConstantBufferUsage::DYNAMIC );

	// 部品が持つのは読んだモデルのハンドル
	auto* render = f.render_of( id );
	EXPECT_EQ( f.resources.resolve_model( render->model ), f.resources.models.get< blue_sky::graphics::Model >( "box-5x5x5" ) );
	EXPECT_EQ( render->object_constant_buffer.get(), f.resources.buffers[ 0 ] );
}

TEST( ObjectRenderResourcesTest, 読めないモデルの物はハンドルが空で描かない )
{
	FactoryFixture f;
	f.resources.missing_models.insert( "box-5x5x5" );

	const auto id = f.factory.create_translation_object( core::Vector( 0.f, 0.f, 0.f ), 1.f, 1.f, 1.f, 1.f );

	EXPECT_TRUE( f.render_of( id )->model.is_null() );
	EXPECT_EQ( f.factory.get_ref( id ).get_model(), nullptr );
}

TEST( ObjectRenderResourcesTest, 自分だけのモデルにすると複製したモデルのハンドルに替わる )
{
	FactoryFixture f;

	const auto id = f.factory.create_translation_object( core::Vector( 0.f, 0.f, 0.f ), 1.f, 1.f, 1.f, 1.f );
	GameObjectRef o = f.factory.get_ref( id );
	auto* shared = o.get_model();

	f.resources.calls.clear();
	EXPECT_TRUE( o.make_model_unique() );

	const std::vector< std::string > expected = { "clone_model_with_shaders" };
	EXPECT_EQ( f.resources.calls, expected );

	ASSERT_NE( o.get_model(), nullptr );
	EXPECT_NE( o.get_model(), shared );
	EXPECT_EQ( f.resources.resolve_model( f.render_of( id )->model ), o.get_model() );
}

TEST( ObjectRenderResourcesTest, 敵は見た目ごとのテクスチャを名前で読みハンドルで持つ )
{
	FactoryFixture f;
	f.resources.loadable_textures = { "robot", "robot-warn", "robot-tender" };	// robot-error は読めない

	const auto id = f.factory.create_object( "robot" );
	ASSERT_TRUE( id );

	// 4 つの見た目のテクスチャを順に読む
	std::vector< std::string > textures;

	for ( const auto& c : f.resources.calls )
	{
		if ( c.rfind( "get_texture ", 0 ) == 0 )
		{
			textures.push_back( c );
		}
	}

	const std::vector< std::string > expected = { "get_texture robot", "get_texture robot-warn", "get_texture robot-error", "get_texture robot-tender" };
	EXPECT_EQ( textures, expected );

	const auto* robot = f.manager.find( * id )->get_component< blue_sky::ecs::RobotBehaviorComponent >();
	ASSERT_NE( robot, nullptr );

	EXPECT_EQ( f.resources.resolve_texture( robot->textures[ 0 ] ), f.resources.textures.get( "robot" ) );
	EXPECT_EQ( f.resources.resolve_texture( robot->textures[ 1 ] ), f.resources.textures.get( "robot-warn" ) );
	EXPECT_TRUE( robot->textures[ 2 ].is_null() );		// 読めなかったテクスチャは空のハンドル
	EXPECT_FALSE( robot->textures[ 3 ].is_null() );
}
