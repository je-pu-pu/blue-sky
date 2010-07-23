#include "StageSelectScene.h"
#include "Input.h"
#include "SoundManager.h"

#include "Direct3D9.h"
#include "Direct3D9Mesh.h"
#include "Direct3D9TextureManager.h"
#include "DirectX.h"

#include <game/Sound.h>

#include <win/Rect.h>

#include <common/exception.h>

// #include <boost/filesystem/operations.hpp>

namespace blue_sky
{

StageSelectScene::StageSelectScene( const GameMain* game_main )
	: Scene( game_main )
	, page_( 0 )
	, sprite_texture_( 0 )
	, ok_( 0 )
	, cursor_src_rect_( win::Rect::Size( 0, 702, 92, 136 ) )
	, left_allow_src_rect_( win::Rect::Size( 256, 704, 82, 126 ) )
	, right_allow_src_rect_( win::Rect::Size( 384, 704, 86, 126 ) )
	, stage_src_rect_( 0, 0, 512, 512 )
{
	sprite_texture_ = direct_3d()->getTextureManager()->load( "sprite", "media/image/title.png" );

	ok_ = sound_manager()->get_sound( "ok" );

	// boost::filesystem::directory_iterator end;
	// for ( boost::filesystem::directory_iterator i( boost::filesystem::path( "stage/" ) ); i != end; ++i )

	std::list< std::string > stage_name_list;

	WIN32_FIND_DATA find_data;

	HANDLE find_handle = FindFirstFile( "media/stage/stage-0-*", & find_data );

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

	int n = 0;

	for ( std::list< std::string >::iterator i = stage_name_list.begin(); i != stage_name_list.end(); ++i )
	{
		Stage* stage = new Stage();
		stage->name = *i;
		stage->rect = get_stage_dst_rect( stage, n );

		try
		{
			stage->texture = direct_3d()->getTextureManager()->load( i->c_str(), ( std::string( "media/stage/image/" ) + *i + ".png" ).c_str() );
		}
		catch ( ... )
		{
			stage->texture = direct_3d()->getTextureManager()->load( i->c_str(), "media/stage/image/stage-default.png" );
		}

		stage_list_.push_back( stage );

		n++;
	}
}

StageSelectScene::~StageSelectScene()
{
	direct_3d()->getTextureManager()->unload( "sprite" );

	for ( StageList::const_iterator i = stage_list_.begin(); i != stage_list_.end(); ++i )
	{
		Stage* stage = *i;

		delete stage;
	}

	stage_list_.clear();
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
			ok_->play( false );
		}
		if ( is_mouse_on_right_allow() )
		{
			ok_->play( false );
		}

		if ( Stage* stage = get_pointed_stage() )
		{
			ok_->play( false );

			if ( stage->name == "stage-0-4" )
			{
				set_stage_name( "" );
			}
			else if ( stage->name == "stage-0-3" )
			{
				set_next_scene( "ending" );
				return;
			}
			else
			{
				set_stage_name( stage->name );
			}

			set_next_scene( "game_play" );
		}
	}
}

/**
 * •`‰æ
 */
bool StageSelectScene::render()
{
	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->BeginScene() );
	DIRECT_X_FAIL_CHECK( direct_3d()->getDevice()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 0x11, 0xAA, 0xFF ), 1.f, 0 ) );
	DIRECT_X_FAIL_CHECK( direct_3d()->getSprite()->Begin( D3DXSPRITE_ALPHABLEND ) );

	D3DXMATRIXA16 t, s, transform;

	// Stage
	int n = 0;

	for ( StageList::const_iterator i = stage_list_.begin(); i != stage_list_.end(); ++i )
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

bool StageSelectScene::has_prev_page() const
{
	return false;
}

bool StageSelectScene::has_next_page() const
{
	return true;
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