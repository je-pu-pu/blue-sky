#include "pch.h"

// RenderSystem がモデルをハンドルから引くことのテスト。解放されたモデルの物体は描かず、落ちない
#include <core/ecs/EntityManager.h>
#include <core/ecs/system/RenderSystem.h>
#include <core/graphics/GraphicsManager.h>
#include <core/graphics/RenderTargetTexture.h>
#include <core/graphics/InstanceBuffer.h>
#include <core/ResourceManager.h>

namespace {

using core::ecs::TransformComponent;
using core::ecs::ModelComponent;
using core::ecs::RenderComponent;
using core::ecs::RenderSystem;
using core::graphics::Mesh;
using core::graphics::Shader;
using core::graphics::PixelFormat;
using core::graphics::InstanceBuffer;
using core::Color;
using core::Vector;
using core::Matrix;

/// モデルの偽物（描いた回数を数える）
class CountModel : public core::graphics::Model
{
public:
	mutable int render_count = 0;

	Mesh* get_mesh() override { return nullptr; }
	const Mesh* get_mesh() const override { return nullptr; }
	void set_mesh( Mesh* ) override { }
	Shader* get_shader_at( size_t ) override { return nullptr; }
	const Shader* get_shader_at( size_t ) const override { return nullptr; }
	void set_shader_at( size_t, Shader* ) override { }
	size_t get_shader_count() const override { return 0; }
	void render() const override { render_count++; }
};

class FakeRenderTargetTexture : public core::graphics::RenderTargetTexture
{
public:
	uint_t get_width() const override { return 1; }
	uint_t get_height() const override { return 1; }
	uint_t get_multi_sample_count() const override { return 1; }
	uint_t get_multi_sample_quality() const override { return 0; }
	void bind_to_ds( uint_t ) const override { }
	void bind_to_ps( uint_t ) const override { }
	void activate() override { }
	void clear( const Color& ) override { }
};

class FakeInstanceBuffer : public core::graphics::InstanceBuffer
{
public:
	void update( const Matrix*, size_t ) override { }
	void bind_to_vs() const override { }
	void unbind_from_vs() const override { }
	size_t get_current_count() const override { return 0; }
};

class FakeConstantBuffer : public core::graphics::ConstantBuffer
{
public:
	FakeConstantBuffer() : ConstantBuffer( Usage::DYNAMIC ) { }
	void update( const void* ) const override { }
	void bind_to_vs() const override { }
	void bind_to_hs() const override { }
	void bind_to_ds() const override { }
	void bind_to_cs() const override { }
	void bind_to_gs() const override { }
	void bind_to_ps() const override { }
};

/// core の描画の窓口の偽物（モデルは本物と同じ ResourceManager で持つ。シェーダーは無い）
class FakeCoreGraphicsManager : public core::graphics::GraphicsManager
{
public:
	core::ResourceManager< core::graphics::Model > models;

	std::unique_ptr< ConstantBuffer > create_constant_buffer( size_t, int, ConstantBufferUsage ) override { return std::make_unique< FakeConstantBuffer >(); }
	std::unique_ptr< InstanceBuffer > create_instance_buffer() override { return std::make_unique< FakeInstanceBuffer >(); }
	Texture* load_texture( const char_t* ) override { return nullptr; }
	core::graphics::Model* resolve_model( ModelHandle h ) const override { return models.resolve( h ); }
	RenderTargetTexture* create_render_target_texture() override { return new FakeRenderTargetTexture(); }
	RenderTargetTexture* create_render_target_texture( PixelFormat ) override { return new FakeRenderTargetTexture(); }
	RenderTargetTexture* create_render_target_texture( PixelFormat, const RenderTargetTextureOptions& ) override { return new FakeRenderTargetTexture(); }
	BackBufferTexture* get_back_buffer_texture() override { return nullptr; }
	void set_render_target( RenderTarget* ) override { }
	void set_default_render_target( bool ) override { }
	void setup_rendering() override { }
	void render_background() const override { }
	const InputLayout* get_input_layout( const char_t* ) const override { return nullptr; }
	EffectTechnique* get_effect_technique( const char_t* ) const override { return nullptr; }
	void set_input_layout( const char_t* ) const override { }
	void set_input_layout( const InputLayout* ) const override { }
	void set_primitive_topology( PrimitiveTopology ) const override { }
	void render_technique( const char_t*, const std::function< void () >& ) const override { }
	void render_technique( const EffectTechnique*, const std::function< void () >& ) const override { }
	void set_current_object_constant_buffer( const ConstantBuffer* ) const override { }
	void set_current_skinning_constant_buffer( const ConstantBuffer* ) const override { }
	void set_post_effect_shader( Shader* ) override { }
	void render_post_effect( Texture* ) override { }
	void render_post_effect( Texture*, RenderTarget* ) override { }
	void render_fader() const override { }
	void clear_debug_bullet() const override { }
	void render_debug_bullet() const override { }
	void count_draw() const override { }
	Vector get_camera_position() const override { return Vector( 0.f, 0.f, 0.f ); }
	Vector get_camera_forward() const override { return Vector( 0.f, 0.f, 1.f ); }
};

struct RenderSystemFixture
{
	FakeCoreGraphicsManager graphics;
	core::ecs::EntityManager entity_manager;

	RenderSystemFixture()
	{
		entity_manager.add_system< RenderSystem >( 1000, & graphics, nullptr );	// シェーダーが無いので時間は使わない
	}

	void add_entity( core::graphics::ModelHandle model )
	{
		auto* e = entity_manager.create_entity();
		e->add_component< TransformComponent >()->transform.set_identity();
		e->add_component< RenderComponent >();
		e->add_component< ModelComponent >()->model = model;
	}
};

TEST( RenderSystemHandleTest, ハンドルのモデルを描く )
{
	RenderSystemFixture f;
	auto* model = f.graphics.models.create_named< CountModel >( "box" );

	f.add_entity( f.graphics.models.handle_of( model ) );
	f.entity_manager.render();

	EXPECT_EQ( model->render_count, 1 );
}

TEST( RenderSystemHandleTest, 解放したモデルの物体は描かず落ちない )
{
	RenderSystemFixture f;
	auto* box = f.graphics.models.create_named< CountModel >( "box" );
	auto* ball = f.graphics.models.create_named< CountModel >( "ball" );

	f.add_entity( f.graphics.models.handle_of( box ) );
	f.add_entity( f.graphics.models.handle_of( ball ) );
	f.entity_manager.render();

	f.graphics.models.remove_named( "box" );	// 解放する（以前の生ポインタはここで無効なアドレスを指した）
	f.graphics.models.flush_releases();		// 解放はフレームの終わり
	f.entity_manager.render();					// 落ちない

	EXPECT_EQ( ball->render_count, 2 );			// 残っているモデルは描き続ける
}

TEST( RenderSystemHandleTest, 空のハンドルの物体は描かない )
{
	RenderSystemFixture f;

	f.add_entity( core::graphics::ModelHandle() );
	f.entity_manager.render();	// 落ちない（以前は nullptr のモデルを描こうとして落ちた）

	SUCCEED();
}

} // namespace
