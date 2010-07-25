#include "StoryTextScene.h"
#include "Direct3D9.h"
#include "Direct3D9Font.h"
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
{
	load_story_text_file( file_name );

	text_y_target_ = static_cast< float >( -direct_3d()->getFont()->get_text_height( text_.c_str() ) );
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
 * ƒƒCƒ“ƒ‹[ƒvˆ—
 *
 */
void StoryTextScene::update()
{
	if ( input()->press( Input::A ) )
	{
		text_y_ -= text_scroll_speed_ * 8.f;
	}
	else
	{
		text_y_ -= text_scroll_speed_;
	}

	if ( text_y_ < text_y_target_ )
	{
		set_next_scene( next_scene_name_ );
	}	
}

/**
 * •`‰æ
 */
bool StoryTextScene::render()
{ 
	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->BeginScene() );
	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB( 0x00, 0x00, 0x00 ), 1.f, 0 ) );

	direct_3d()->getFont()->draw_text_center( 0, static_cast< int >( text_y_ ), text_.c_str(), D3DCOLOR_ARGB( 127, 255, 255, 255 ) );

	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->EndScene() );

	return true;
}

} // namespace blue_sky