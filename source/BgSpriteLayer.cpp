#include "BgSpriteLayer.h"

#include <core/graphics/Direct3D11/Direct3D11Texture.h>
#include <core/math.h>

BgSpriteLayer::BgSpriteLayer( const char_t* name, const Texture* texture )
	: name_( name )
	, texture_( texture )
	, src_rect_( 0, 0, texture->get_width(), texture->get_height() )
	, dst_rect_( 0, 0, texture->get_width(), texture->get_height() )
	, translation_( Vector3( 0.f, 0.f, 0.f ), Vector3( 0.f, 0.f, 0.f ), Vector3( 0.f, 0.f, 0.f ) )
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
