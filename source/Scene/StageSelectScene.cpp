/**
 * @todo get_direct_3d() の使用をやめて GraphicsManager を使うようにする。
 */

#include "StageSelectScene.h"

#include <blue_sky/Input.h>
#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/graphics/Fader.h>

/// @todo 抽象化する
#include <core/graphics/Direct3D11/Direct3D11.h>
#include <core/graphics/Direct3D11/Sprite.h>

#include <core/sound/SoundManager.h>
#include <core/sound/Sound.h>

#include <game/Sound.h>
#include <game/Config.h>

#include <win/Rect.h>

#include <common/exception.h>
#include <common/serialize.h>

#include <boost/filesystem.hpp>

#include <random>

namespace blue_sky
{

StageSelectScene::StageSelectScene()
	: page_( 0 )
	, stage_count_( 0 )
	, is_story_completed_( false )
	, sprite_texture_( 0 )
	, bg_texture_( 0 )
	, ok_( 0 )
	, click_( 0 )
	, cursor_src_rect_( win::Rect::Size( 0, 702, 92, 136 ) )
	, left_arrow_src_rect_( win::Rect::Size( 256, 704, 82, 126 ) )
	, right_arrow_src_rect_( win::Rect::Size( 384, 704, 86, 126 ) )
	, stage_src_rect_( 0, 0, 512, 512 )
{
	page_ = get_save_data()->get( "stage-select.page", 0 );

	check_story_completed();

	circle_src_rect_list_.push_back( win::Rect::Size( 256, 512, 128, 140 ) );
	circle_src_rect_list_.push_back( win::Rect::Size( 384, 512, 160, 130 ) );
	circle_src_rect_list_.push_back( win::Rect::Size( 576, 512, 144, 114 ) );
	circle_src_rect_list_.push_back( win::Rect::Size( 768, 512, 146, 120 ) );

	face_src_rect_list_.push_back( win::Rect::Size( 576, 384, 64, 64 ) );
	face_src_rect_list_.push_back( win::Rect::Size( 640, 384, 64, 64 ) );
	face_src_rect_list_.push_back( win::Rect::Size( 704, 384, 64, 64 ) );
	face_src_rect_list_.push_back( win::Rect::Size( 768, 384, 64, 64 ) );

	sprite_texture_ = get_graphics_manager()->load_named_texture( "sprite", "media/image/title.png" );
	bg_texture_ = get_graphics_manager()->load_named_texture( "bg", "media/texture/cloth.png" );

	ok_ = get_sound_manager()->get_sound( "ok" );
	click_ = get_sound_manager()->get_sound( "click" );

	update_stage_list();

	get_graphics_manager()->get_fader()->set_color( Color::White );
	get_graphics_manager()->get_fader()->full_out();
}

StageSelectScene::~StageSelectScene()
{
	/// @todo 直す
#if false
	// get_graphics_manager()->unload_texture( "sprite" );
	get_graphics_manager()->unload_texture( "bg" );
#endif

	clear_stage_list();
}

/**
 * メインループ処理
 *
 */
void StageSelectScene::update()
{
	get_graphics_manager()->get_fader()->fade_in();

	if ( get_input()->push( Input::Button::A ) )
	{
		if ( is_mouse_on_left_arrow() )
		{
			update_page( page_ - 1 );

			ok_->play( false );
		}
		else if ( is_mouse_on_right_arrow() )
		{
			update_page( page_ + 1 );

			ok_->play( false );
		}
		else if ( Stage* stage = get_pointed_stage() )
		{
			ok_->play( false );

			set_next_stage_name( stage->name );

			if ( boost::filesystem::exists( string_t( "media/stage/" ) + stage->name + ".intro" ) )
			{
				set_next_scene( "stage_intro" );
			}
			else
			{
				set_next_scene( "game_play" );
			}
		}
		else
		{
			click_->play( false );
		}
	}
}

/**
 * 描画
 */
void StageSelectScene::render()
{
	Color bg_color = page_ < get_max_story_page() ? Color::from_256( 0xFF, 0xAA, 0x11 ) : Color::from_256( 0x99, 0xEE, 0xFF );

	get_direct_3d()->set_default_render_target();
	get_direct_3d()->set_default_viewport();

	get_direct_3d()->clear_default_view( bg_color );

	get_direct_3d()->get_sprite()->begin();

	render_technique( "|sprite", [this]
	{
		// render_bg()
		{
			get_direct_3d()->get_sprite()->draw( win::Rect::Size( 0, 0, get_width(), get_height() ), bg_texture_ );
		}
	} );

	Direct3D::Matrix t, s, transform;

	// Stage
	RectList::const_iterator j = circle_src_rect_list_.begin();
	RectList::const_iterator k = face_src_rect_list_.begin();

	for ( StageList::const_iterator i = stage_list_.begin(); i != stage_list_.end(); ++i, ++j, ++k )
	{
		Stage* stage = *i;

		// Direct3D::Vector center( stage_src_rect_.width() * 0.5f, stage_src_rect_.height() * 0.5f, 0.f );

		win::Rect dst_rect = stage->rect;

		// float dx = dst_rect.left() + dst_rect.width() * 0.5f - ( get_width() / 2 );
		// float dy = dst_rect.top() + dst_rect.height() * 0.5f - ( get_height() / 2 );

		int offset = 0;
		Direct3D::Color frame_color = Direct3D::Color::White;

		if ( stage == get_pointed_stage() )
		{
			if ( get_input()->press( Input::Button::A ) )
			{
				offset = 3;
			}
			frame_color = Direct3D::Color::from_hex( 0xFFAA11FF );
		}

		win::Rect white_src_rect = win::Rect::Size( 0, 512, 128, 128 );
		// Direct3D::Vector white_center( white_src_rect.width() * 0.5f, white_src_rect.height() * 0.5f, 0.f );

		// float scale = static_cast< float >( dst_rect.width() ) / static_cast< float >( stage_src_rect_.width() );
		// float frame_scale = ( dst_rect.width() + 10.f ) / white_src_rect.width();
		const win::Rect frame_scale_rect( -5, -5, +5, +5 );

		// shadow
		get_direct_3d()->get_sprite()->draw( dst_rect + frame_scale_rect + win::Point( -5, 5 ), sprite_texture_, white_src_rect, Direct3D::Color::from_hex( 0x00000099 ) );

		// white
		get_direct_3d()->get_sprite()->draw( dst_rect + frame_scale_rect + win::Point( -offset, offset ), sprite_texture_, white_src_rect, frame_color );

		// stage
		get_direct_3d()->get_sprite()->draw( dst_rect + win::Point( -offset, offset ), stage->texture, stage_src_rect_, Direct3D::Color::White );

		// circle
		if ( stage->cleared )
		{
			win::Point circle_dst_point = win::Point( dst_rect.right() - j->width(), dst_rect.bottom() - j->height() ) + win::Point( -offset, offset );
			get_direct_3d()->get_sprite()->draw( circle_dst_point, sprite_texture_, *j, Direct3D::Color::from_hex( 0xFFFFFF99 ) );
		}

		// face
		if ( stage->completed )
		{
			// face
			if ( true )
			{
				win::Point circle_dst_point = win::Point( dst_rect.right() - j->width() + ( j->width() - k->width() ) / 2, dst_rect.bottom() - j->height() + ( j->height() - k->height() ) / 2 ) + win::Point( -offset, offset );
				get_direct_3d()->get_sprite()->draw( circle_dst_point, sprite_texture_, *k, Direct3D::Color::from_hex( 0xFFFFFF99 ) );
			}

			// medal
			win::Rect medal_src_rect = win::Rect::Size( 832, 384, 64, 64 );
			win::Point medal_dst_point = win::Point( dst_rect.left(), dst_rect.bottom() - medal_src_rect.height() ) + win::Point( -offset, offset );
			get_direct_3d()->get_sprite()->draw( medal_dst_point, sprite_texture_, medal_src_rect, Direct3D::Color::from_hex( 0xFFFFFF99 ) );
		}
	}

	// Arrow
	if ( has_prev_page() )
	{
		win::Rect src_rect = left_arrow_src_rect_;

		if ( is_mouse_on_left_arrow() )
		{
			src_rect.left() += 256;
			src_rect.right() += 256;
		}

		win::Point arrow_dst_point = win::Point( get_margin(), ( get_height() - src_rect.height() ) / 2 );
		get_direct_3d()->get_sprite()->draw( arrow_dst_point, sprite_texture_, src_rect.get_rect() );
	}
	if ( has_next_page() )
	{
		win::Rect src_rect = right_arrow_src_rect_;

		if ( is_mouse_on_right_arrow() )
		{
			src_rect.left() += 256;
			src_rect.right() += 256;
		}

		win::Point arrow_dst_point( get_width() - get_margin() - src_rect.width(), ( get_height() - src_rect.height() ) / 2 );
		get_direct_3d()->get_sprite()->draw( arrow_dst_point, sprite_texture_, src_rect.get_rect() );
	}

	// Cursor
	{
		win::Rect src_rect = cursor_src_rect_;

		if ( get_input()->press( Input::Button::A ) )
		{
			src_rect.left() += 128;
			src_rect.right() += 128;
		}

		win::Point cursor_dst_point( get_input()->get_mouse_x(), get_input()->get_mouse_y() );
		get_direct_3d()->get_sprite()->draw( cursor_dst_point, sprite_texture_, src_rect.get_rect() );
	}

	get_direct_3d()->get_sprite()->end();

	render_fader();
}

void StageSelectScene::update_page( int page )
{
	page_ = page;

	update_stage_list();

	get_save_data()->set( "stage-select.page", page );
}

void StageSelectScene::clear_stage_list()
{
	for ( StageList::const_iterator i = stage_list_.begin(); i != stage_list_.end(); ++i )
	{
		Stage* stage = *i;

		/// @todo 直す
#if 0
		get_graphics_manager()->unload_texture( stage->name.c_str() );
#endif

		delete stage;
	}

	stage_list_.clear();
}

void StageSelectScene::update_stage_list()
{
	clear_stage_list();

	std::list< string_t > stage_name_list;

	WIN32_FIND_DATA find_data;

	string_t file_pattern;
	
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
	string_t last_stage_name;

	for ( auto i = stage_name_list.begin(); i != stage_name_list.end(); ++i )
	{
		auto stage_name = *i;
		stage_name.resize( stage_name.find_first_of( "." ) );

		// ストーリー用ステージでは、前のステージをクリアしていないと、このステージは一覧に含まない
		if ( is_story_page() && ! last_stage_name.empty() )
		{
			if ( get_save_data()->get( ( get_stage_prefix_by_page( page_ ) + "." + last_stage_name ).c_str(), 0 ) == 0 )
			{
				break;
			}
		}
		if ( stage_name == common::serialize( get_max_story_page() - 1 ) + "-" + common::serialize( get_max_stage_per_page() ) )
		{
			// final stage
			if ( ! is_final_stage_open() )
			{
				continue;
			}
		}

		last_stage_name = stage_name;

		Stage* stage = new Stage();
		stage->name = stage_name;
		stage->rect = get_stage_dst_rect( stage, n );
		stage->cleared = get_save_data()->get( ( get_stage_prefix_by_page( page_ ) + "." + stage->name ).c_str(), 0 ) != 0;
		stage->completed = get_save_data()->get( ( get_stage_prefix_by_page( page_ ) + "." + stage->name ).c_str(), 0 ) == 2;

		try
		{
			stage->texture = get_graphics_manager()->load_named_texture( stage->name.c_str(), ( get_stage_dir_name_by_page( page_ ) + stage->name + ".png" ).c_str() );
		}
		catch ( ... )
		{
			stage->texture = get_graphics_manager()->load_named_texture( stage->name.c_str(), "media/stage/default.png" );
		}

		stage_list_.push_back( stage );

		n++;

		if ( n >= get_max_stage_per_page() )
		{
			break;
		}
	}

	std::random_device seed_generator;
	std::mt19937 random_generator( seed_generator() );

	std::shuffle( circle_src_rect_list_.begin(), circle_src_rect_list_.end(), random_generator );
	std::shuffle( face_src_rect_list_.begin(), face_src_rect_list_.end(), random_generator );
}

string_t StageSelectScene::get_stage_dir_name_by_page( int page )
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

string_t StageSelectScene::get_stage_prefix_by_page( int page )
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

void StageSelectScene::check_story_completed()
{
	is_story_completed_ = false;

	for ( int p = 0; p < get_max_story_page(); p++ )
	{
		for ( int s = 0; s < get_max_stage_per_page(); s++ )
		{
			std::string stage_name = common::serialize( p ) + "-" + common::serialize( s );

			if ( get_save_data()->get( ( get_stage_prefix_by_page( p ) + "." + stage_name ).c_str(), 0 ) < 2 )
			{
				return;
			}
		}
	}

	is_story_completed_ = true;
}

bool StageSelectScene::is_final_stage_open() const
{
	for ( int s = 0; s < get_max_stage_per_page(); s++ )
	{
		std::string stage_name = common::serialize( get_max_story_page() - 1 ) + "-" + common::serialize( s );

		if ( get_save_data()->get( ( get_stage_prefix_by_page( get_max_story_page() - 1 ) + "." + stage_name ).c_str(), 0 ) == 0 )
		{
			return false;
		}
	}

	return true;
}

bool StageSelectScene::has_prev_page() const
{
	return page_ > 0;
}

bool StageSelectScene::has_next_page() const
{
	if ( page_ == get_max_story_page() - 1 )
	{
		// stroy last page
		return is_story_completed_;
	}
	else if ( page_ < get_max_story_page() )
	{
		// story normal page
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

bool StageSelectScene::is_mouse_on_left_arrow() const
{
	if ( ! has_prev_page() ) return false;

	win::Rect rect = win::Rect::Size( get_margin(), ( get_height() - left_arrow_src_rect_.height() ) / 2, left_arrow_src_rect_.width(), left_arrow_src_rect_.height() );

	if ( get_input()->get_mouse_x() <  rect.left()   ) return false;
	if ( get_input()->get_mouse_x() >= rect.right()  ) return false;
	if ( get_input()->get_mouse_y() <  rect.top()    ) return false;
	if ( get_input()->get_mouse_y() >= rect.bottom() ) return false;

	return true;
}

bool StageSelectScene::is_mouse_on_right_arrow() const
{
	if ( ! has_next_page() ) return false;

	win::Rect rect = win::Rect::Size( get_width() - get_margin() - right_arrow_src_rect_.width(), ( get_height() - right_arrow_src_rect_.height() ) / 2, right_arrow_src_rect_.width(), right_arrow_src_rect_.height() );

	if ( get_input()->get_mouse_x() <  rect.left()   ) return false;
	if ( get_input()->get_mouse_x() >= rect.right()  ) return false;
	if ( get_input()->get_mouse_y() <  rect.top()    ) return false;
	if ( get_input()->get_mouse_y() >= rect.bottom() ) return false;

	return true;
}

StageSelectScene::Stage* StageSelectScene::get_pointed_stage() const
{
	for ( StageList::const_iterator i = stage_list_.begin(); i != stage_list_.end(); ++i )
	{
		Stage* stage = *i;

		if ( get_input()->get_mouse_x() <  stage->rect.left()   ) continue;
		if ( get_input()->get_mouse_x() >= stage->rect.right()  ) continue;
		if ( get_input()->get_mouse_y() <  stage->rect.top()    ) continue;
		if ( get_input()->get_mouse_y() >= stage->rect.bottom() ) continue;

		return stage;
	}

	return 0;
}

win::Rect StageSelectScene::get_stage_dst_rect( const Stage* /* stage */, int n ) const
{
	const int stage_x_count = 2;
	const int stage_y_count = 2;

	int w = get_width() - get_margin() - left_arrow_src_rect_.width() - get_margin() * 2 - right_arrow_src_rect_.width() - get_margin();
	int h = get_height() - get_margin() * 2;

	int stage_interval_w = w / stage_x_count;
	int stage_interval_h = h / stage_y_count;

	int dst_w = 512;
	
	dst_w = std::min( dst_w, stage_interval_w - get_margin() - 20 );
	dst_w = std::min( dst_w, stage_interval_h - get_margin() - 20 );

	int x = n % stage_x_count;
	int y = n / stage_x_count;

	int dx = get_margin() + left_arrow_src_rect_.width() + get_margin() + stage_interval_w * x + stage_interval_w / 2;
	int dy = get_margin() + stage_interval_h * y + stage_interval_h / 2;

	return win::Rect::Size( dx - dst_w / 2, dy - dst_w / 2, dst_w, dst_w );
}

} // namespace blue_sky