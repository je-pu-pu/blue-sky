#pragma once

// 描画の小さな窓口（doc/描画の窓口の分割.md）の偽物。呼ばれ方を記録する
#include <blue_sky/graphics/RenderSettings.h>
#include <blue_sky/graphics/ResourceLoader.h>
#include <blue_sky/graphics/Canvas2D.h>
#include <blue_sky/graphics/Model.h>
#include <blue_sky/graphics/Line.h>

#include <core/ResourceManager.h>
#include <core/graphics/ConstantBuffer.h>
#include <core/graphics/Texture.h>
#include <core/graphics/Sprite.h>
#include <core/graphics/TextStyle.h>

#include <win/Rect.h>

#include <cstring>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
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

/// テクスチャの偽物（大きさだけ持つ）
class FakeTexture : public core::graphics::Texture
{
public:
	uint_t width = 1;
	uint_t height = 1;

	FakeTexture() = default;
	FakeTexture( uint_t w, uint_t h ) : width( w ), height( h ) { }

	uint_t get_width() const override { return width; }
	uint_t get_height() const override { return height; }
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
	void render_instanced( uint_t ) const override { }
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
	core::graphics::TextureHandle get_texture_handle_of( const Texture* t ) const override { return textures.handle_of( t ); }

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

/// win::Rect を "左,上,右,下" にする
inline std::string to_string( const win::Rect& r )
{
	std::ostringstream ss;
	ss << r.left() << "," << r.top() << "," << r.right() << "," << r.bottom();
	return ss.str();
}

/// Sprite の偽物（呼ばれた順に記録する。記録先は Canvas2D の偽物と共有して、文字との順番も分かるようにする）
class FakeSprite : public core::graphics::Sprite
{
	std::vector< std::string >& calls_;

protected:
	void draw( const Rect*, const Texture*, const Rect*, const core::Color* ) override { calls_.push_back( "sprite.draw" ); }
	void draw_uv_impl( const Rect& dst, const Texture*, const UVRect&, const core::Color&, AlphaMode mode ) override
	{
		calls_.push_back( "sprite.draw_uv " + to_string( dst ) + ( mode == AlphaMode::PREMULTIPLIED ? " 乗算済み" : " ストレート" ) );
	}

public:
	explicit FakeSprite( std::vector< std::string >& calls ) : calls_( calls ) { }

	void set_transform( const core::Matrix& ) override { }
	void set_ortho_offset( float_t ) override { }
	void begin() override { calls_.push_back( "sprite.begin" ); }
	void draw_color( const Rect& r, const core::Color& ) override { calls_.push_back( "sprite.draw_color " + to_string( r ) ); }
	void end() override { calls_.push_back( "sprite.end" ); }
};

/**
 * 2D を描く窓口の偽物
 *
 * 文字の幅は「文字数 × フォントサイズ × 0.5」、行の高さは「フォントサイズ × 1.25」と決めて返す。
 * UI の画像は images に名前（"ui:…"）を入れたものだけ読める（無ければ例外。本物と同じ）
 */
class FakeCanvas2D : public blue_sky::graphics::Canvas2D
{
public:
	int		screen_width = 1920;
	int		screen_height = 1080;
	float_t	render_target_width = 1920.f;
	float_t	render_target_height = 1080.f;
	float_t	default_font_size = 64.f;

	mutable std::vector< std::string >	calls;
	mutable FakeSprite					sprite{ calls };

	core::ResourceManager< core::graphics::Texture >	textures;
	std::set< std::string >								images;		///< 読める UI の画像の名前（"ui:…"）

	int get_screen_width() const override { return screen_width; }
	int get_screen_height() const override { return screen_height; }
	float_t get_render_target_width() const override { return render_target_width; }
	float_t get_render_target_height() const override { return render_target_height; }

	core::graphics::Sprite* get_sprite() const override { return & sprite; }

	void queue_text( float_t left, float_t top, float_t right, float_t bottom, const char_t* text, const core::graphics::TextStyle& style ) const override
	{
		std::ostringstream ss;
		ss << "queue_text " << text << " " << left << "," << top << "," << right << "," << bottom << " 大きさ " << style.font_size << " 縁 " << style.outline_width;
		calls.push_back( ss.str() );
	}

	void flush_text() const override { calls.push_back( "flush_text" ); }

	float_t measure_text_width( const char_t* text, float_t font_size ) const override
	{
		calls.push_back( "measure_text_width " + std::to_string( static_cast< int >( font_size ) ) );
		return static_cast< float_t >( std::strlen( text ) ) * font_size * 0.5f;
	}

	float_t measure_text_line_height( float_t font_size ) const override { return font_size * 1.25f; }
	float_t get_default_font_size() const override { return default_font_size; }

	core::graphics::Texture* load_named_texture_premultiplied( const char_t* name, const char_t* file_name ) override
	{
		calls.push_back( std::string( "load " ) + name + " " + file_name );

		if ( auto* t = textures.get( name ) )
		{
			return t;
		}

		if ( ! images.count( name ) )
		{
			throw std::runtime_error( std::string( "読めません: " ) + file_name );
		}

		return textures.create_named< FakeTexture >( name, 200u, 100u );
	}
};

} // namespace test_graphics
