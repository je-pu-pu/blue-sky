#include "StoryTextScene.h"
#include "Direct3D9.h"
#include "Direct3D9Font.h"
#include "Direct3D9TextureManager.h"
#include "DirectX.h"
#include "SoundManager.h"
#include "Input.h"

#include <fstream>

namespace blue_sky
{

StoryTextScene::StoryTextScene( const GameMain* game_main, const char* file_name, const char* next_scene_name )
	: Scene( game_main )
	, text_y_( static_cast< float >( get_height() ) )
	, text_y_target_( 0.f )
	, text_scroll_speed_( 0.5f )
	, next_scene_name_( next_scene_name )
	, bg_texture_( 0 )
{
	load_story_text_file( file_name );

	text_y_target_ = static_cast< float >( -direct_3d()->getFont()->get_text_height( text_.c_str() ) );

	// bg_texture_ = direct_3d()->getTextureManager()->load( "bg", "media/image/test.jpg" );
	bg_texture_ = direct_3d()->getTextureManager()->load( "bg", "media/image/town.jpg" );
}

StoryTextScene::~StoryTextScene()
{

}

void StoryTextScene::load_story_text_file( const char* file_name )
{
	std::ifstream in( file_name );
	
	if ( ! in.good() )
	{
		set_next_scene( next_scene_name_ );
		return;
	}

	while ( in.good() )
	{
		std::string line;
		std::getline( in, line );

		text_ += line + "\n";
	}
}

/**
 * ���C�����[�v����
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
}

/**
 * �`��
 */
bool StoryTextScene::render()
{ 
	static float bg_scale_ = 1.f;

	bg_scale_ += 0.0001f;

	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->BeginScene() );
	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB( 0x00, 0x00, 0x00 ), 1.f, 0 ) );

	// bg
	DIRECT_X_FAIL_CHECK( direct_3d()->getSprite()->Begin( D3DXSPRITE_ALPHABLEND ) );

	RECT src_rect = { 0, 0, 1024, 1024 };
	D3DXVECTOR3 center( ( src_rect.right - src_rect.left ) * 0.5f, ( src_rect.bottom - src_rect.top ) * 0.5f, 0.f );
	D3DXVECTOR3 position( get_width() * 0.5f, get_height() * 0.5f, 0.f );

	D3DXMATRIXA16 transform;
	D3DXMATRIXA16 s, t;

	D3DXMatrixScaling( & s, bg_scale_, bg_scale_, 1.f );
	D3DXMatrixTranslation( & t, get_width() * 0.5f, get_height() * 0.5f, 0.f );

	transform = s * t;
	direct_3d()->getSprite()->SetTransform( & transform );
	direct_3d()->getSprite()->Draw( bg_texture_, & src_rect, & center, 0, 0xFFFFFFFF );

	DIRECT_X_FAIL_CHECK( direct_3d()->getSprite()->End() );

	// text
	direct_3d()->getFont()->draw_text_center( 0, static_cast< int >( text_y_ ), text_.c_str(), D3DCOLOR_ARGB( 127, 255, 255, 255 ) );

	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->EndScene() );

	return true;
}

} // namespace blue_sky