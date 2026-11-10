#pragma once

// 描画の小さな窓口（doc/描画の窓口の分割.md）の偽物。呼ばれ方を記録する
#include <blue_sky/graphics/RenderSettings.h>
#include <blue_sky/graphics/ResourceLoader.h>
#include <blue_sky/graphics/Model.h>
#include <blue_sky/graphics/Line.h>

#include <core/ResourceManager.h>
#include <core/graphics/ConstantBuffer.h>
#include <core/graphics/Texture.h>

#include <cstring>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace test_graphics
{

/// 定数バッファの偽物（最後に書いた中身と、書いた回数を持つ）
class FakeConstantBuffer : public core::graphics::ConstantBuffer
{
public:
	size_t						size;
	int							slot;
	mutable int					update_count = 0;
	mutable std::vector< char >	data;

	FakeConstantBuffer( size_t s, int sl, Usage usage ) : ConstantBuffer( usage ), size( s ), slot( sl ), data( s ) { }

	void update( const void* d ) const override { std::memcpy( data.data(), d, size ); update_count++; }

	/// 最後に書いた中身を型として読む
	template< typename T > const T& as() const { return * reinterpret_cast< const T* >( data.data() ); }

	void bind_to_vs() const override { }
	void bind_to_hs() const override { }
	void bind_to_ds() const override { }
	void bind_to_cs() const override { }
	void bind_to_gs() const override { }
	void bind_to_ps() const override { }
};

/// テクスチャの偽物（中身は無い）
class FakeTexture : public core::graphics::Texture
{
public:
	uint_t get_width() const override { return 1; }
	uint_t get_height() const override { return 1; }
	uint_t get_multi_sample_count() const override { return 1; }
	uint_t get_multi_sample_quality() const override { return 0; }
	void bind_to_ds( uint_t ) const override { }
	void bind_to_ps( uint_t ) const override { }
};

/// 線の偽物（色だけ使う）
class FakeLine : public blue_sky::graphics::Line
{
protected:
	void create_vertex_buffer() override { }
	void create_index_buffer() override { }
	void create_texture() override { }
public:
	void render_part( int ) const override { }
};

/**
 * 資源を読む・引く窓口の偽物
 *
 * モデル・テクスチャは本物と同じ ResourceManager で持つので、ハンドルの振る舞い（解放したら引けない）も本物と同じ。
 * load_model は知らない名前なら空のモデルを作る（missing_models の名前は読めない）。get_texture は loadable_textures の名前だけ読める
 */
class FakeResourceLoader : public blue_sky::graphics::ResourceLoader
{
public:
	using Model = blue_sky::graphics::Model;
	using Texture = core::graphics::Texture;

	core::ResourceManager< core::graphics::Model >		models;
	core::ResourceManager< core::graphics::Texture >	textures;

	std::vector< std::string >	calls;					///< 読み込み・作成の呼び出し（引くだけの関数は記録しない）
	std::set< std::string >		missing_models;			///< load_model で読めない名前
	std::set< std::string >		loadable_textures;		///< get_texture で読める名前
	std::vector< FakeConstantBuffer* >	buffers;		///< 作った定数バッファ（持ち主は受け取った側）

	Model* add_model( const char* name ) { return models.create_named< Model >( name ); }

	Model* resolve_model( core::graphics::ModelHandle h ) const override { return static_cast< Model* >( models.resolve( h ) ); }
	core::graphics::ModelHandle get_model_handle( const core::graphics::Model* m ) const override { return models.handle_of( m ); }
	Texture* resolve_texture( core::graphics::TextureHandle h ) const override { return textures.resolve( h ); }
	core::graphics::TextureHandle get_texture_handle( const char_t* name ) const override { return textures.get_handle( name ); }

	Model* load_model( const char_t* name ) override
	{
		calls.push_back( std::string( "load_model " ) + name );

		if ( Model* m = models.get< Model >( name ) )
		{
			return m;
		}

		return missing_models.count( name ) ? nullptr : add_model( name );
	}

	Model* clone_model_with_shaders( const Model* m ) override
	{
		calls.push_back( "clone_model_with_shaders" );
		return models.create< Model >( * m );
	}

	Texture* get_texture( const char_t* name ) override
	{
		calls.push_back( std::string( "get_texture " ) + name );

		if ( Texture* t = textures.get( name ) )
		{
			return t;
		}

		return loadable_textures.count( name ) ? textures.create_named< FakeTexture >( name ) : nullptr;
	}

	std::unique_ptr< core::graphics::ConstantBuffer > create_constant_buffer( size_t size, int slot, core::graphics::ConstantBufferUsage usage ) override
	{
		calls.push_back( "create_constant_buffer " + std::to_string( slot ) );

		auto b = std::make_unique< FakeConstantBuffer >( size, slot, usage );
		buffers.push_back( b.get() );
		return b;
	}
};

/// 描画の設定の偽物（呼ばれた順に "関数名 値" を記録し、最後の値も持つ）
class FakeRenderSettings : public blue_sky::graphics::RenderSettings
{
public:
	std::vector< std::string >	calls;

	core::Color		ambient_color;
	core::Color		shadow_color;
	core::Color		shadow_paper_color;
	float_t			drawing_accent = -1.f;

	void set_sky_box( const char_t* name ) override { calls.push_back( std::string( "set_sky_box " ) + name ); }
	void unset_sky_box() override { calls.push_back( "unset_sky_box" ); }
	void set_ground( const char_t* name ) override { calls.push_back( std::string( "set_ground " ) + name ); }

	void set_ambient_color( const core::Color& c ) override { ambient_color = c; calls.push_back( "set_ambient_color" ); }
	void set_shadow_color( const core::Color& c ) override { shadow_color = c; calls.push_back( "set_shadow_color" ); }
	void set_shadow_paper_color( const core::Color& c ) override { shadow_paper_color = c; calls.push_back( "set_shadow_paper_color" ); }
	void set_drawing_accent( float_t a ) override { drawing_accent = a; calls.push_back( "set_drawing_accent" ); }

	void set_drawing_line_type( int_t t ) override { calls.push_back( "set_drawing_line_type " + std::to_string( t ) ); }
	void set_paper_texture_type( int_t t ) override { calls.push_back( "set_paper_texture_type " + std::to_string( t ) ); }
};

} // namespace test_graphics
