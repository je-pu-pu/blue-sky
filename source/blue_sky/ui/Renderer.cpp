#include "Renderer.h"

#include <blue_sky/DebugLog.h>
#include <blue_sky/graphics/GraphicsManager.h>

#include <core/graphics/Sprite.h>
#include <core/graphics/Texture.h>
#include <core/ui/PixelSnap.h>

#include <common/exception.h>

#include <win/Rect.h>

#include <algorithm>
#include <cmath>
#include <exception>

namespace blue_sky::ui
{

Renderer::Renderer( GraphicsManager* gm )
	: gm_( gm )
{
}

core::ui::VirtualScreen Renderer::get_virtual_screen() const
{
	return core::ui::compute_virtual_screen( static_cast< float_t >( gm_->get_screen_width() ), static_cast< float_t >( gm_->get_screen_height() ), DESIGN_WIDTH, DESIGN_HEIGHT );
}

float_t Renderer::get_scale() const
{
	return get_virtual_screen().scale;
}

Renderer::Rect Renderer::to_physical( const Rect& r ) const
{
	const float_t s = get_scale();
	return Rect( r.x * s, r.y * s, r.w * s, r.h * s );
}

float_t Renderer::physical_to_virtual_x( float_t px ) const
{
	return px / get_scale();
}

float_t Renderer::physical_to_virtual_y( float_t py ) const
{
	return py / get_scale();
}

void Renderer::draw_text( const Rect& rect, const char_t* text, const core::graphics::TextStyle& style )
{
	if ( ! text || ! *text )
	{
		return;
	}

	const float_t s = get_scale();
	const Rect p = to_physical( rect );

	core::graphics::TextStyle scaled = style;
	scaled.outline_width *= s;
	scaled.font_size = ( style.font_size > 0.f ? style.font_size : gm_->get_default_font_size() ) * s;

	begin_text_mode();
	gm_->queue_text( p.x, p.y, p.right(), p.bottom(), text, scaled );
}

void Renderer::begin_sprite_mode()
{
	if ( mode_ == BatchMode::SPRITE )
	{
		return;
	}

	if ( mode_ == BatchMode::TEXT )
	{
		gm_->flush_text();		// 先に積んだ文字を描く（後から描く Sprite が上に重なるように）
	}

	gm_->get_sprite()->begin();
	mode_ = BatchMode::SPRITE;
}

void Renderer::begin_text_mode()
{
	if ( mode_ == BatchMode::TEXT )
	{
		return;
	}

	if ( mode_ == BatchMode::SPRITE )
	{
		gm_->get_sprite()->end();
	}

	mode_ = BatchMode::TEXT;
}

void Renderer::flush()
{
	if ( mode_ == BatchMode::SPRITE )
	{
		gm_->get_sprite()->end();
	}
	else if ( mode_ == BatchMode::TEXT )
	{
		gm_->flush_text();
	}

	mode_ = BatchMode::NONE;
}

win::Rect Renderer::to_pixel_rect( const Rect& r ) const
{
	// 丸めの規則は core::ui::to_pixel_rect（テスト用の Mock と共有）
	const core::ui::PixelRect p = core::ui::to_pixel_rect( r, get_scale(), 0.f, 0.f );
	return win::Rect( static_cast< LONG >( p.left ), static_cast< LONG >( p.top ), static_cast< LONG >( p.right ), static_cast< LONG >( p.bottom ) );
}

float_t Renderer::get_pixel_scale() const
{
	return get_scale();
}

Renderer::Rect Renderer::snap_to_pixels( const Rect& r ) const
{
	return core::ui::snap_rect_to_pixels( r, get_scale(), 0.f, 0.f );
}

string_t Renderer::normalize_image_name( const string_t& image )
{
	string_t name = image;

	for ( auto& c : name )
	{
		if ( c == '\\' )
		{
			c = '/';
		}
	}

	while ( name.size() >= 2 && name[ 0 ] == '.' && name[ 1 ] == '/' )
	{
		name.erase( 0, 2 );
	}

	return name;
}

void Renderer::draw_rect( const Rect& rect, const Color& color )
{
	const win::Rect px = to_pixel_rect( rect );

	if ( px.right() <= px.left() || px.bottom() <= px.top() )
	{
		return;
	}

	begin_sprite_mode();
	gm_->get_sprite()->draw_color( px, color );
}

void Renderer::fill_screen( const Color& color )
{
	begin_sprite_mode();
	gm_->get_sprite()->draw_color( win::Rect( 0, 0, gm_->get_screen_width(), gm_->get_screen_height() ), color );
}

const core::graphics::Texture* Renderer::find_image( const string_t& image ) const
{
	if ( image.empty() )
	{
		return nullptr;
	}

	if ( auto i = images_.find( image ); i != images_.end() )
	{
		return i->second;
	}

	// 初回だけ読み込む。失敗も覚えておき、毎フレーム読み直さない
	// 名前を正規化してからテクスチャの名前にする（"ui/a.png" と "./ui/a.png" を二重に読まない）
	// UI 用の読み込み（乗算済みアルファ・ミップなし）を使う。描画も乗算済み用の technique で行う
	const core::graphics::Texture* texture = nullptr;
	const string_t name = normalize_image_name( image );
	const string_t path = string_t( IMAGE_DIRECTORY ) + name;

	try
	{
		texture = gm_->load_named_texture_premultiplied( ( string_t( "ui:" ) + name ).c_str(), path.c_str() );
	}
	catch ( const common::exception< std::string >& e )
	{
		debug_log( "[UI] 画像を読み込めません: " + path + " (" + e.data() + ")" );
	}
	catch ( const std::exception& e )
	{
		debug_log( "[UI] 画像を読み込めません: " + path + " (" + e.what() + ")" );
	}
	catch ( ... )
	{
		// 例外の型はエンジン内で統一されていない（DIRECT_X_FAIL_CHECK は別の型を投げる）。UI の画像 1 枚のために落とさない
		debug_log( "[UI] 画像を読み込めません: " + path + " (不明な例外)" );
	}

	if ( texture && ( texture->get_width() == 0 || texture->get_height() == 0 ) )
	{
		texture = nullptr;
	}

	images_[ image ] = texture;
	return texture;
}

bool Renderer::get_image_size( const string_t& image, float_t& width, float_t& height ) const
{
	const auto* texture = find_image( image );

	if ( ! texture )
	{
		return false;
	}

	width = static_cast< float_t >( texture->get_width() );
	height = static_cast< float_t >( texture->get_height() );
	return true;
}

void Renderer::draw_image( const Rect& rect, const string_t& image, const Rect& source, const Color& tint )
{
	const auto* texture = find_image( image );

	if ( ! texture )
	{
		return;
	}

	const win::Rect px = to_pixel_rect( rect );

	if ( px.right() <= px.left() || px.bottom() <= px.top() )
	{
		return;
	}

	const float_t tw = static_cast< float_t >( texture->get_width() );
	const float_t th = static_cast< float_t >( texture->get_height() );

	core::graphics::Sprite::UVRect uv;
	uv.u0 = source.x / tw;
	uv.v0 = source.y / th;
	uv.u1 = source.right() / tw;
	uv.v1 = source.bottom() / th;

	begin_sprite_mode();
	gm_->get_sprite()->draw_uv( px, texture, uv, tint, core::graphics::Sprite::AlphaMode::PREMULTIPLIED );
}

float_t Renderer::measure_text_width( const char_t* text, float_t font_size ) const
{
	// 物理ピクセルで測って仮想座標に戻す（フォントのヒンティング差を含めて実際の描画と一致させる）
	const float_t s = get_scale();

	if ( s <= 0.f )
	{
		return 0.f;
	}

	const float_t size = ( font_size > 0.f ? font_size : gm_->get_default_font_size() ) * s;
	return gm_->measure_text_width( text, size ) / s;
}

float_t Renderer::measure_line_height( float_t font_size ) const
{
	const float_t s = get_scale();

	if ( s <= 0.f )
	{
		return 0.f;
	}

	const float_t size = ( font_size > 0.f ? font_size : gm_->get_default_font_size() ) * s;
	return gm_->measure_text_line_height( size ) / s;
}

} // namespace blue_sky::ui
