#include "StoryTextScene.h"
#include "Direct3D11.h"
#include "Direct3D11Vector.h"
#include "Direct3D11Matrix.h"
#include "Direct3D11Sprite.h"
#include "Direct3D11Effect.h"
#include "Direct3D11Fader.h"
#include "DirectWrite.h"
#include "Direct3D11TextureManager.h"
#include "DirectX.h"
#include "SoundManager.h"
#include "Sound.h"
#include "Input.h"

#include <win/Rect.h>

#include <common/string.h>

#include <fstream>
#include <sstream>

namespace blue_sky
{

StoryTextScene::StoryTextScene( const GameMain* game_main, const char* file_name, const char* next_scene_name )
	: Scene( game_main )
	, text_y_( static_cast< float >( get_height() ) )
	, text_y_target_( 0.f )
	, text_scroll_speed_( 0.5f )
	, text_color_( Direct3D::Color::from_256( 255, 255, 255, 127 ) )
	, text_border_color_( Direct3D::Color::from_256( 0, 0, 0, 127 ) )
	, next_scene_name_( next_scene_name )
	, sprite_texture_( 0 )
	, bg_texture_( 0 )
	, bg_width_( 0 )
	, bg_height_( 0 )
	, bg_scale_( 1.f, 1.f, 0.001f )
	, bgm_( 0 )
	, sound_( 0 )
	, is_skipped_( false )
{
	get_sound_manager()->stop_all();

	load_story_text_file( file_name );

	text_y_target_ = static_cast< float >( -get_direct_3d()->getFont()->get_text_height( text_.c_str(), static_cast< float >( get_width() ), static_cast< float >( get_height() ) ) );
	
	sprite_texture_ = get_direct_3d()->getTextureManager()->load( "sprite", "media/image/title.png" );

	if ( ! bg_texture_ )
	{
		bg_texture_ = get_direct_3d()->getTextureManager()->load( "bg", "media/image/story-bg-default.jpg" );
	}

	if ( bg_width_ == 0 && bg_height_ == 0 )
	{
		D3D11_TEXTURE2D_DESC texture_2d_desc;
		get_direct_3d()->getTexture2dDescByTexture( bg_texture_, & texture_2d_desc );

		bg_width_ = texture_2d_desc.Width;
		bg_height_ = texture_2d_desc.Height;
	}

	get_direct_3d()->getFader()->full_out();
}

StoryTextScene::~StoryTextScene()
{
	get_direct_3d()->getTextureManager()->unload( "sprite" );
	get_direct_3d()->getTextureManager()->unload( "bg" );

	get_sound_manager()->stop_all();
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
			text_ += common::convert_to_wstring( line ) + L"\n";
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

			get_direct_3d()->getTextureManager()->unload( "bg" );
			bg_texture_ = get_direct_3d()->getTextureManager()->load( "bg", ( std::string( "media/image/" + bg_file_name ) ).c_str() );
		}
		else if ( name == "bg-scale" )
		{
			ss >> bg_scale_.value() >> bg_scale_.target_value() >> bg_scale_.speed();
		}
		else if ( name == "bgm" )
		{
			std::string bgm_name;
			float_t volume = 1.f;

			ss >> bgm_name >> volume;

			bgm_ = get_sound_manager()->load_music( "bgm", bgm_name.c_str() );
			bgm_->set_volume( volume );
			bgm_->play( false );
		}
		else if ( name == "text-color" )
		{
			uint_t hex;

			ss >> std::hex >> hex;

			text_color_ = Direct3D::Color::from_hex( hex );
		}
		else if( name == "text-border-color" )
		{
			uint_t hex;

			ss >> std::hex >> hex;

			text_border_color_ = Direct3D::Color::from_hex( hex );
		}
		else if ( name == "text" )
		{
			in_text = true;
		}
		else if ( name == "sound" )
		{
			std::string sound_name;
			ss >> sound_name;
			sound_ = get_sound_manager()->load( "sound", sound_name.c_str() );
			sound_->play( false );
		}
		else if ( name == "next-scene" )
		{
			ss >> next_scene_name_;
		}
		else if ( name == "next-stage-name" )
		{
			ss >> next_stage_name_;
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

	if ( ! text_.empty() )
	{
		if ( get_input()->press( Input::A ) )
		{
			speed *= 8.f;
		}
		if ( get_input()->press( Input::B ) )
		{
			speed *= 8.f;
		}
	
		text_y_ -= speed;

		if ( text_y_ < text_y_target_ )
		{
			set_next_scene( next_scene_name_ );
		}
	}
	else if ( sound_ )
	{
		if ( ! is_skipped_ )
		{
			if ( get_input()->push( Input::A ) || ! sound_->is_playing() )
			{
				is_skipped_ = true;
			}
		}
		else
		{
			sound_->fade_out();

			if ( bgm_ )
			{
				bgm_->fade_out();
			}

			get_direct_3d()->getFader()->fade_out();
		}

		if ( ! sound_->is_playing() && ( ! bgm_ || ! bgm_->is_playing() ) && get_direct_3d()->getFader()->is_full_out() )
		{
			set_next_scene( next_scene_name_ );

			if ( ! next_stage_name_.empty() )
			{
				set_next_stage_name( next_stage_name_ );
			}
		}
	}

	if ( ! is_skipped_ )
	{
		get_direct_3d()->getFader()->fade_in();
	}

	bg_scale_.chase();
}

/**
 * •`‰æ
 */
void StoryTextScene::render()
{
	get_direct_3d()->clear();
	get_direct_3d()->getSprite()->begin();

	Direct3D::Matrix transform;
	Direct3D::Matrix s, t;

	Direct3D::EffectTechnique* technique = get_direct_3d()->getEffect()->getTechnique( "|sprite" );

	for ( Direct3D::EffectTechnique::PassList::const_iterator i = technique->getPassList().begin(); i != technique->getPassList().end(); ++i )
	{
		( *i )->apply();

		// bg
		{
			win::Rect dst_rect( 0, 0, get_width(), get_height() );

			get_direct_3d()->getSprite()->set_transform( transform );
			get_direct_3d()->getSprite()->draw( dst_rect, bg_texture_ );
		}
	}

	get_direct_3d()->getSprite()->end();

	// text
	get_direct_3d()->begin2D();
	get_direct_3d()->getFont()->begin();
	get_direct_3d()->getFont()->draw_text_center( -1.f, text_y_ - 1.f, static_cast< float >( get_width() ), static_cast< float >( get_height() ), text_.c_str(), text_border_color_ );
	get_direct_3d()->getFont()->draw_text_center( +1.f, text_y_ - 1.f, static_cast< float >( get_width() ), static_cast< float >( get_height() ), text_.c_str(), text_border_color_ );
	get_direct_3d()->getFont()->draw_text_center( -1.f, text_y_ + 1.f, static_cast< float >( get_width() ), static_cast< float >( get_height() ), text_.c_str(), text_border_color_ );
	get_direct_3d()->getFont()->draw_text_center( +1.f, text_y_ + 1.f, static_cast< float >( get_width() ), static_cast< float >( get_height() ), text_.c_str(), text_border_color_ );
	get_direct_3d()->getFont()->draw_text_center( 0.f, text_y_, static_cast< float >( get_width() ), static_cast< float >( get_height() ), text_.c_str(), text_color_ );
	get_direct_3d()->getFont()->end();
	get_direct_3d()->end2D();

	get_direct_3d()->begin3D();
	get_direct_3d()->renderText();

	render_fader();

	// loading ...
	if ( is_skipped_ )
	{
		get_direct_3d()->getSprite()->begin();

		Direct3D::EffectTechnique* technique = get_direct_3d()->getEffect()->getTechnique( "|sprite" );

		for ( Direct3D::EffectTechnique::PassList::const_iterator i = technique->getPassList().begin(); i != technique->getPassList().end(); ++i )
		{
			( *i )->apply();

			win::Rect src_rect = win::Rect::Size( 512, 0, 272, 128 );
			win::Point dst_point( get_width() - src_rect.width(), get_height() - src_rect.height() );

			get_direct_3d()->getSprite()->draw( dst_point, sprite_texture_, src_rect );
		}

		get_direct_3d()->getSprite()->end();
	}

	get_direct_3d()->end3D();
}

} // namespace blue_sky