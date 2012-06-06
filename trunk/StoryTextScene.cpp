#include "StoryTextScene.h"
#include "Direct3D9.h"
#include "Direct3D9Font.h"
#include "Direct3D9TextureManager.h"
#include "DirectX.h"
#include "SoundManager.h"
#include "Sound.h"
#include "Input.h"

#include <win/Rect.h>

#include <fstream>
#include <sstream>

namespace blue_sky
{

StoryTextScene::StoryTextScene( const GameMain* game_main, const char* file_name, const char* next_scene_name )
	: Scene( game_main )
	, text_y_( static_cast< float >( get_height() ) )
	, text_y_target_( 0.f )
	, text_scroll_speed_( 0.5f )
	, text_color_( D3DCOLOR_ARGB( 127, 255, 255, 255 ) )
	, next_scene_name_( next_scene_name )
	, sprite_texture_( 0 )
	, bg_texture_( 0 )
	, bg_width_( 0 )
	, bg_height_( 0 )
	, bg_scale_( 1.f, 1.f, 0.001f )
{
	load_story_text_file( file_name );

	text_y_target_ = static_cast< float >( -direct_3d()->getFont()->get_text_height( text_.c_str() ) );

	sprite_texture_ = direct_3d()->getTextureManager()->load( "sprite", "media/image/title.png" );
	
	if ( ! bg_texture_ )
	{
		bg_texture_ = direct_3d()->getTextureManager()->load( "bg", "media/image/noise.jpg" );
		bg_width_ = 800;
		bg_height_ = 600;
	}

	D3DSURFACE_DESC bg_texture_surface_desc;
	DIRECT_X_FAIL_CHECK( bg_texture_->GetLevelDesc( 0, & bg_texture_surface_desc ) );

	if ( bg_width_ == 0 ) bg_width_ = bg_texture_surface_desc.Width;
	if ( bg_height_ == 0 ) bg_height_ = bg_texture_surface_desc.Height;
}

StoryTextScene::~StoryTextScene()
{
	direct_3d()->getTextureManager()->unload( "sprite" );
	direct_3d()->getTextureManager()->unload( "bg" );
}

void StoryTextScene::load_story_text_file( const char* file_name )
{
	std::ifstream in( file_name );
	
	if ( ! in.good() )
	{
		set_next_scene( next_scene_name_ );
		return;
	}

	bool in_text = false;

	while ( in.good() )
	{
		std::string line;
		std::getline( in, line );

		if ( in_text )
		{
			text_ += line + "\n";
			continue;
		}

		std::stringstream ss;
		
		std::string name;
		std::string value;
		
		ss << line;

		ss >> name;

		if ( name == "bg" )
		{
			std::string bg_file_name;
			ss >> bg_file_name >> bg_width_ >> bg_height_;

			direct_3d()->getTextureManager()->unload( "bg" );
			bg_texture_ = direct_3d()->getTextureManager()->load( "bg", ( std::string( "media/image/" + bg_file_name ) ).c_str() );
		}
		else if ( name == "bg-scale" )
		{
			ss >> bg_scale_.value() >> bg_scale_.target_value() >> bg_scale_.speed();
		}
		else if ( name == "bgm" )
		{
			std::string bgm_name;
			ss >> bgm_name;

			sound_manager()->load_music( "bgm", bgm_name.c_str() )->play( false );
		}
		else if ( name == "text-color" )
		{
			ss >> std::hex >> text_color_;
		}
		else if ( name == "text" )
		{
			in_text = true;
		}
	}
}

/**
 * ƒƒCƒ“ƒ‹[ƒvˆ—
 *
 */
void StoryTextScene::update()
{
	float speed = text_scroll_speed_;

	if ( input()->press( Input::A ) )
	{
		speed *= 8.f;
	}
	if ( input()->press( Input::B ) )
	{
		speed *= 8.f;
	}
	
	text_y_ -= speed;

	if ( text_y_ < text_y_target_ )
	{
		set_next_scene( next_scene_name_ );
	}	

	bg_scale_.chase();
}

/**
 * •`‰æ
 */
bool StoryTextScene::render()
{ 
	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->BeginScene() );
	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB( 0x00, 0x00, 0x00 ), 1.f, 0 ) );

	DIRECT_X_FAIL_CHECK( direct_3d()->getSprite()->Begin( D3DXSPRITE_ALPHABLEND ) );

	D3DXMATRIXA16 transform;
	D3DXMATRIXA16 s, t;

	// bg
	{
		win::Rect src_rect( 0, 0, bg_width_, bg_height_ );
		D3DXVECTOR3 center( src_rect.width() * 0.5f, src_rect.height() * 0.5f, 0.f );
		D3DXVECTOR3 position( get_width() * 0.5f, get_height() * 0.5f, 0.f );

		float x_ratio = static_cast< float >( get_width() ) / static_cast< float >( src_rect.width() );
		float y_ratio = static_cast< float >( get_height() ) / static_cast< float >( src_rect.height() );
		float ratio = std::min( x_ratio, y_ratio );

		D3DXMatrixScaling( & s, bg_scale_.value() * ratio, bg_scale_.value() * ratio, 1.f );
		D3DXMatrixTranslation( & t, get_width() * 0.5f, get_height() * 0.5f, 0.f );

		transform = s * t;
		direct_3d()->getSprite()->SetTransform( & transform );
		direct_3d()->getSprite()->Draw( bg_texture_, & src_rect.get_rect(), & center, 0, 0xFFFFFFFF );
	}

	// loading ...
	if ( ! get_next_scene().empty() )
	{
		win::Rect src_rect = win::Rect::Size( 512, 0, 272, 128 );
		D3DXVECTOR3 center( src_rect.width() * 0.5f, src_rect.height() * 0.5f, 0.f );

		D3DXMatrixTranslation( & transform, get_width() - src_rect.width() * 0.5f, get_height() - src_rect.height() * 0.5f, 0.f );

		direct_3d()->getSprite()->SetTransform( & transform );
		direct_3d()->getSprite()->Draw( sprite_texture_, & src_rect.get_rect(), & center, 0, 0xFFFFFFFF );
	}

	DIRECT_X_FAIL_CHECK( direct_3d()->getSprite()->End() );

	// text
	direct_3d()->getFont()->draw_text_center( 0, static_cast< int >( text_y_ ), text_.c_str(), text_color_ );

	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->EndScene() );

	return true;
}

} // namespace blue_sky