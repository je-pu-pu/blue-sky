#include "Image.h"
#include "Renderer.h"

namespace core::ui
{

bool Image::get_source( const Renderer& renderer, Rect& source ) const
{
	if ( image_.empty() )
	{
		return false;
	}

	float_t w = 0.f;
	float_t h = 0.f;

	if ( ! renderer.get_image_size( image_, w, h ) || w <= 0.f || h <= 0.f )
	{
		return false;
	}

	if ( source_.w > 0.f && source_.h > 0.f )
	{
		// 画像の外にはみ出す指定は画像の中に収める（Brush と同じ扱い。はみ出すと端の画素が引き伸ばされる）
		source = source_.intersection( Rect( 0.f, 0.f, w, h ) );
		return source.w > 0.f && source.h > 0.f;
	}

	source = Rect( 0.f, 0.f, w, h );
	return true;
}

float_t Image::measure_height( float_t width, const UIContext& ctx ) const
{
	if ( height_ > 0.f )
	{
		return height_;
	}

	Rect source;

	if ( ! get_source( ctx.renderer, source ) )
	{
		return 0.f;
	}

	return width * source.h / source.w;
}

void Image::layout( ScaleMode mode, const Rect& rect, const Rect& source, Rect& dst, Rect& src )
{
	dst = rect;
	src = source;

	if ( mode == ScaleMode::STRETCH || rect.w <= 0.f || rect.h <= 0.f || source.w <= 0.f || source.h <= 0.f )
	{
		return;
	}

	const float_t rect_aspect = rect.w / rect.h;
	const float_t source_aspect = source.w / source.h;

	if ( mode == ScaleMode::FIT )
	{
		// 収める : 長い方の辺を合わせ、短い方に余白
		if ( source_aspect > rect_aspect )
		{
			const float_t h = rect.w / source_aspect;
			dst = Rect( rect.x, rect.y + ( rect.h - h ) * 0.5f, rect.w, h );
		}
		else
		{
			const float_t w = rect.h * source_aspect;
			dst = Rect( rect.x + ( rect.w - w ) * 0.5f, rect.y, w, rect.h );
		}
	}
	else
	{
		// 覆う : 描画先はそのまま、画像側を中央で切り取る
		if ( source_aspect > rect_aspect )
		{
			const float_t w = source.h * rect_aspect;
			src = Rect( source.x + ( source.w - w ) * 0.5f, source.y, w, source.h );
		}
		else
		{
			const float_t h = source.w / rect_aspect;
			src = Rect( source.x, source.y + ( source.h - h ) * 0.5f, source.w, h );
		}
	}
}

void Image::render( Renderer& renderer, const UIContext& ) const
{
	Rect source;

	if ( tint_.a() <= 0.f || ! get_source( renderer, source ) )
	{
		return;
	}

	Rect dst;
	Rect src;
	layout( mode_, rect_, source, dst, src );

	if ( dst.w > 0.f && dst.h > 0.f )
	{
		renderer.draw_image( dst, image_, src, tint_ );
	}
}

} // namespace core::ui
