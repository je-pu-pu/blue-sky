#include "StageSelectScene.h"
#include "Input.h"
#include "SoundManager.h"

#include "Direct3D9.h"
#include "Direct3D9Mesh.h"
#include "Direct3D9TextureManager.h"
#include "DirectX.h"

#include <game/Sound.h>
#include <game/Config.h>

#include <win/Rect.h>

#include <common/exception.h>
#include <common/serialize.h>

// #include <boost/filesystem/convenience.hpp>

namespace blue_sky
{

StageSelectScene::StageSelectScene( const GameMain* game_main )
	: Scene( game_main )
	, page_( 0 )
	, stage_count_( 0 )
	, sprite_texture_( 0 )
	, ok_( 0 )
	, cursor_src_rect_( win::Rect::Size( 0, 702, 92, 136 ) )
	, left_allow_src_rect_( win::Rect::Size( 256, 704, 82, 126 ) )
	, right_allow_src_rect_( win::Rect::Size( 384, 704, 86, 126 ) )
	, stage_src_rect_( 0, 0, 512, 512 )
{
	page_ = save_data()->get( "stage-select.page", 0 );

	circle_src_rect_list_.push_back( win::Rect::Size( 256, 512, 128, 140 ) );
	circle_src_rect_list_.push_back( win::Rect::Size( 384, 512, 160, 130 ) );
	circle_src_rect_list_.push_back( win::Rect::Size( 576, 512, 144, 114 ) );
	circle_src_rect_list_.push_back( win::Rect::Size( 768, 512, 146, 120 ) );

	face_src_rect_list_.push_back( win::Rect::Size( 576, 384, 64, 64 ) );
	face_src_rect_list_.push_back( win::Rect::Size( 640, 384, 64, 64 ) );
	face_src_rect_list_.push_back( win::Rect::Size( 704, 384, 64, 64 ) );
	face_src_rect_list_.push_back( win::Rect::Size( 768, 384, 64, 64 ) );

	sprite_texture_ = direct_3d()->getTextureManager()->load( "sprite", "media/image/title.png" );

	ok_ = sound_manager()->load( "ok" );

	update_stage_list();
}

StageSelectScene::~StageSelectScene()
{
	direct_3d()->getTextureManager()->unload( "sprite" );

	clear_stage_list();
}

/**
 * ƒƒCƒ“ƒ‹[ƒvˆ—
 *
 */
void StageSelectScene::update()
{
	if ( input()->push( Input::A ) )
	{
		if ( is_mouse_on_left_allow() )
		{
			update_page( page_ - 1 );

			ok_->play( false );
		}
		if ( is_mouse_on_right_allow() )
		{
			update_page( page_ + 1 );

			ok_->play( false );
		}

		if ( Stage* stage = get_pointed_stage() )
		{
			ok_->play( false );

			if ( stage->name == "1-1" )
			{
				set_next_stage_name( "" );
			}
			else if ( stage->name == "1-2" )
			{
				set_next_scene( "ending" );
				return;
			}
			else
			{
				set_next_stage_name( stage->name );
			}

			set_next_scene( "stage_intro" );
		}
	}
}

/**
 * •`‰æ
 */
bool StageSelectScene::render()
{
	D3DCOLOR bg_color = page_ < get_max_story_page() ? D3DCOLOR_XRGB( 0x11, 0xAA, 0xFF ) : D3DCOLOR_XRGB( 0x99, 0xEE, 0xFF );

	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->BeginScene() );
	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, bg_color, 1.f, 0 ) );
	DIRECT_X_FAIL_CHECK( direct_3d()->getSprite()->Begin( D3DXSPRITE_ALPHABLEND ) );

	D3DXMATRIXA16 t, s, transform;

	// Stage
	int n = 0;

	RectList::const_iterator j = circle_src_rect_list_.begin();
	RectList::const_iterator k = face_src_rect_list_.begin();

	for ( StageList::const_iterator i = stage_list_.begin(); i != stage_list_.end(); ++i, ++j, ++k )
	{
		Stage* stage = *i;

		D3DXVECTOR3 center( stage_src_rect_.width() * 0.5f, stage_src_rect_.height() * 0.5f, 0.f );

		win::Rect dst_rect = stage->rect;

		float dx = dst_rect.left() + dst_rect.width() * 0.5f;
		float dy = dst_rect.top() + dst_rect.height() * 0.5f;

		float offset = 0.f;
		D3DCOLOR frame_color = 0xFFFFFFFF;

		if ( stage == get_pointed_stage() )
		{
			if ( input()->press( Input::A ) )
			{
				offset = 3.f;
			}
			frame_color = 0xFFFFAA11;
		}

		win::Rect white_src_rect = win::Rect::Size( 0, 512, 128, 128 );
		D3DXVECTOR3 white_center( white_src_rect.width() * 0.5f, white_src_rect.height() * 0.5f, 0.f );

		float scale = static_cast< float >( dst_rect.width() ) / static_cast< float >( stage_src_rect_.width() );
		float frame_scale = ( dst_rect.width() + 10.f ) / white_src_rect.width();

		// shadow
		D3DXMatrixTranslation( & t, dx - 5, dy + 5, 0.f );
		D3DXMatrixScaling( & s, frame_scale, frame_scale, frame_scale );
		transform = s * t;

		direct_3d()->getSprite()->SetTransform( & transform );
		direct_3d()->getSprite()->Draw( sprite_texture_, & white_src_rect.get_rect(), & white_center, 0, 0x99000000 );

		// white
		D3DXMatrixTranslation( & t, dx - offset, dy + offset, 0.f );
		D3DXMatrixScaling( & s, frame_scale, frame_scale, frame_scale );
		transform = s * t;

		direct_3d()->getSprite()->SetTransform( & transform );
		direct_3d()->getSprite()->Draw( sprite_texture_, & white_src_rect.get_rect(), & white_center, 0, frame_color );

		// Stage
		D3DXMatrixTranslation( & t, dx - offset, dy + offset, 0.f );
		D3DXMatrixScaling( & s, scale, scale, scale );
		transform = s * t;

		direct_3d()->getSprite()->SetTransform( & transform );
		direct_3d()->getSprite()->Draw( stage->texture, & stage_src_rect_.get_rect(), & center, 0, 0xFFFFFFFF );

		// Circle
		if ( stage->cleared )
		{
			D3DXVECTOR3 circle_center( j->width() * 0.5f, j->height() * 0.5f, 0.f );

			D3DXMatrixTranslation( & t, dx + ( dst_rect.width() - j->width() ) * 0.5f - offset, dy + ( dst_rect.height() - j->height() ) * 0.5f + offset, 0.f );
			transform = t;

			direct_3d()->getSprite()->SetTransform( & transform );
			direct_3d()->getSprite()->Draw( sprite_texture_, & j->get_rect(), & circle_center, 0, 0x99FFFFFF );
		}

		// Face
		if ( stage->completed )
		{
			D3DXVECTOR3 face_center( k->width() * 0.5f, k->height() * 0.5f, 0.f );

			D3DXMatrixTranslation( & t, dx + ( dst_rect.width() - j->width() ) * 0.5f - offset, dy + ( dst_rect.height() - j->height() ) * 0.5f + offset, 0.f );
			transform = t;

			direct_3d()->getSprite()->SetTransform( & transform );
			direct_3d()->getSprite()->Draw( sprite_texture_, & k->get_rect(), & face_center, 0, 0x99FFFFFF );
		}

		n++;
	}

	// Allow
	if ( has_prev_page() )
	{
		win::Rect src_rect = left_allow_src_rect_;

		D3DXVECTOR3 center( src_rect.width() * 0.5f, src_rect.height() * 0.5f, 0.f );

		if ( is_mouse_on_left_allow() )
		{
			src_rect.left() += 256;
			src_rect.right() += 256;
		}

		D3DXMatrixTranslation( & transform, 0.f + src_rect.width() * 0.5f + get_margin(), get_height() * 0.5f, 0.f );

		direct_3d()->getSprite()->SetTransform( & transform );
		direct_3d()->getSprite()->Draw( sprite_texture_, & src_rect.get_rect(), & center, 0, 0xFFFFFFFF );
	}
	if ( has_next_page() )
	{
		win::Rect src_rect = right_allow_src_rect_;

		D3DXVECTOR3 center( src_rect.width() * 0.5f, src_rect.height() * 0.5f, 0.f );

		if ( is_mouse_on_right_allow() )
		{
			src_rect.left() += 256;
			src_rect.right() += 256;
		}

		D3DXMatrixTranslation( & transform, get_width() - src_rect.width() * 0.5f - get_margin(), get_height() * 0.5f, 0.f );

		direct_3d()->getSprite()->SetTransform( & transform );
		direct_3d()->getSprite()->Draw( sprite_texture_, & src_rect.get_rect(), & center, 0, 0xFFFFFFFF );
	}

	// Cursor
	{
		win::Rect src_rect = cursor_src_rect_;

		if ( input()->press( Input::A ) )
		{
			src_rect.left() += 128;
			src_rect.right() += 128;
		}

		D3DXVECTOR3 center( src_rect.width() * 0.5f, src_rect.height() * 0.5f, 0.f );
		
		D3DXMatrixTranslation( & transform, static_cast< float >( input()->get_mouse_x() ) + src_rect.width() * 0.5f - 5.f, static_cast< float >( input()->get_mouse_y() ) + src_rect.height() * 0.5f - 5.f, 0.f );

		direct_3d()->getSprite()->SetTransform( & transform );
		direct_3d()->getSprite()->Draw( sprite_texture_, & src_rect.get_rect(), & center, 0, 0xFFFFFFFF );
	}

	DIRECT_X_FAIL_CHECK( direct_3d()->getSprite()->End() );
	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->EndScene() );

	return true;
}

void StageSelectScene::update_page( int page )
{
	page_ = page;

	update_stage_list();

	save_data()->set( "stage-select.page", page );
}

void StageSelectScene::clear_stage_list()
{
	for ( StageList::const_iterator i = stage_list_.begin(); i != stage_list_.end(); ++i )
	{
		Stage* stage = *i;

		direct_3d()->getTextureManager()->unload( stage->name.c_str() );

		delete stage;
	}

	stage_list_.clear();
}

void StageSelectScene::update_stage_list()
{
	clear_stage_list();

	std::list< std::string > stage_name_list;

	WIN32_FIND_DATA find_data;

	std::string file_pattern;
	
	if ( page_ < get_max_story_page() )
	{
		file_pattern = get_stage_dir_name_by_page( page_ ) + common::serialize( page_ ) + "-*.stage";
	}
	else
	{
		file_pattern = get_stage_dir_name_by_page( page_ ) + "*.stage";
	}
	
	HANDLE find_handle = FindFirstFile( file_pattern.c_str(), & find_data );

	if ( find_handle  != INVALID_HANDLE_VALUE )
	{
		while ( true )
		{
			stage_name_list.push_back( find_data.cFileName );

			if ( ! FindNextFile( find_handle, & find_data ) )
			{
				break;
			}
		}

		FindClose( find_handle );
	}

	stage_count_ = stage_name_list.size();

	int n = 0;

	for ( std::list< std::string >::iterator i = stage_name_list.begin(); i != stage_name_list.end(); ++i )
	{
		Stage* stage = new Stage();
		stage->name = *i;
		stage->name.resize( stage->name.find_first_of( "." ) );
		stage->rect = get_stage_dst_rect( stage, n );
		stage->cleared = save_data()->get( ( get_stage_prefix_by_page( page_ ) + "." + stage->name ).c_str(), 0 ) != 0;
		stage->completed = save_data()->get( ( get_stage_prefix_by_page( page_ ) + "." + stage->name ).c_str(), 0 ) == 2;

		try
		{
			stage->texture = direct_3d()->getTextureManager()->load( stage->name.c_str(), ( get_stage_dir_name_by_page( page_ ) + stage->name + ".png" ).c_str() );
		}
		catch ( ... )
		{
			stage->texture = direct_3d()->getTextureManager()->load( stage->name.c_str(), "media/stage/default.png" );
		}

		stage_list_.push_back( stage );

		n++;

		if ( n >= get_max_stage_per_page() )
		{
			break;
		}
	}

	std::random_shuffle( circle_src_rect_list_.begin(), circle_src_rect_list_.end() );
	std::random_shuffle( face_src_rect_list_.begin(), face_src_rect_list_.end() );
}

std::string StageSelectScene::get_stage_dir_name_by_page( int page )
{
	if ( page < get_max_story_page() )
	{
		return "media/stage/";
	}
	else
	{
		return "media/stage_ext/";
	}
}

std::string StageSelectScene::get_stage_prefix_by_page( int page )
{
	if ( page < get_max_story_page() )
	{
		return "stage";
	}
	else
	{
		return "stage_ext";
	}
}

bool StageSelectScene::has_prev_page() const
{
	return page_ > 0;
}

bool StageSelectScene::has_next_page() const
{
	if ( page_ < get_max_story_page() )
	{
		for ( StageList::const_iterator i = stage_list_.begin(); i != stage_list_.end(); ++i )
		{
			Stage* stage = *i;

			if ( ! stage->cleared )
			{
				return false;
			}
		}

		return true;
	}
	else
	{
		return ( page_ - get_max_story_page() + 1 ) * get_max_stage_per_page() < stage_count_;
	}
}

bool StageSelectScene::is_mouse_on_left_allow() const
{
	if ( ! has_prev_page() ) return false;

	win::Rect rect = win::Rect::Size( get_margin(), ( get_height() - left_allow_src_rect_.height() ) / 2, left_allow_src_rect_.width(), left_allow_src_rect_.height() );

	if ( input()->get_mouse_x() <  rect.left()   ) return false;
	if ( input()->get_mouse_x() >= rect.right()  ) return false;
	if ( input()->get_mouse_y() <  rect.top()    ) return false;
	if ( input()->get_mouse_y() >= rect.bottom() ) return false;

	return true;
}

bool StageSelectScene::is_mouse_on_right_allow() const
{
	if ( ! has_next_page() ) return false;

	win::Rect rect = win::Rect::Size( get_width() - get_margin() - right_allow_src_rect_.width(), ( get_height() - right_allow_src_rect_.height() ) / 2, right_allow_src_rect_.width(), right_allow_src_rect_.height() );

	if ( input()->get_mouse_x() <  rect.left()   ) return false;
	if ( input()->get_mouse_x() >= rect.right()  ) return false;
	if ( input()->get_mouse_y() <  rect.top()    ) return false;
	if ( input()->get_mouse_y() >= rect.bottom() ) return false;

	return true;
}

StageSelectScene::Stage* StageSelectScene::get_pointed_stage() const
{
	for ( StageList::const_iterator i = stage_list_.begin(); i != stage_list_.end(); ++i )
	{
		Stage* stage = *i;

		if ( input()->get_mouse_x() <  stage->rect.left()   ) continue;
		if ( input()->get_mouse_x() >= stage->rect.right()  ) continue;
		if ( input()->get_mouse_y() <  stage->rect.top()    ) continue;
		if ( input()->get_mouse_y() >= stage->rect.bottom() ) continue;

		return stage;
	}

	return 0;
}

win::Rect StageSelectScene::get_stage_dst_rect( const Stage* stage, int n ) const
{
	const int stage_x_count = 2;
	const int stage_y_count = 2;

	int w = get_width() - get_margin() - left_allow_src_rect_.width() - get_margin() * 2 - right_allow_src_rect_.width() - get_margin();
	int h = get_height() - get_margin() * 2;

	int stage_interval_w = w / stage_x_count;
	int stage_interval_h = h / stage_y_count;

	int dst_w = 512;
	
	dst_w = std::min( dst_w, stage_interval_w - get_margin() - 20 );
	dst_w = std::min( dst_w, stage_interval_h - get_margin() - 20 );

	int x = n % stage_x_count;
	int y = n / stage_x_count;

	int dx = get_margin() + left_allow_src_rect_.width() + get_margin() + stage_interval_w * x + stage_interval_w / 2;
	int dy = get_margin() + stage_interval_h * y + stage_interval_h / 2;

	return win::Rect::Size( dx - dst_w / 2, dy - dst_w / 2, dst_w, dst_w );
}

} // namespace blue_sky