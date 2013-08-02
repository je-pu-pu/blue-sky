#include "BgSpriteLayer.h"

#include "memory.h"

BgSpriteLayer::BgSpriteLayer( const char_t* name, const Texture* texture )
	: name_( name )
	, texture_( texture )
	, src_rect_( 0, 0, texture->get_width(), texture->get_height() )
	, dst_rect_( 0, 0, texture->get_width(), texture->get_height() )
	, translation_( vector3( 0.f, 0.f, 0.f ), vector3( 0.f, 0.f, 0.f ), vector3( 0.f, 0.f, 0.f ) )
	, rotation_( 0.f, 0.f, 0.f )
	, scale_( 1.f, 1.f, 0.f )
	, color_( Color::White, Color::White, Color( 0.f, 0.f, 0.f, 0.f ) )
{
	
}

BgSpriteLayer::~BgSpriteLayer()
{

}

void BgSpriteLayer::update()
{
	translation_.chase();
	rotation_.chase();
	scale_.chase();
	color_.chase();
}

namespace math
{

template<> vector3 chase( vector3 value, vector3 target, vector3 speed )
{
	value.x() = chase( value.x(), target.x(), speed.x() );
	value.y() = chase( value.y(), target.y(), speed.y() );
	value.z() = chase( value.z(), target.z(), speed.z() );

	return value;
}

template<> BgSpriteLayer::Color chase( BgSpriteLayer::Color value, BgSpriteLayer::Color target, BgSpriteLayer::Color speed )
{
	value.r() = chase( value.r(), target.r(), speed.r() );
	value.g() = chase( value.g(), target.g(), speed.g() );
	value.b() = chase( value.b(), target.b(), speed.b() );
	value.a() = chase( value.a(), target.a(), speed.a() );

	return value;
}

}