#include "StoryTextScene.h"

#include "BgSpriteLayer.h"

#include "Input.h"
#include "SoundManager.h"
#include "Sound.h"

#include <blue_sky/graphics/GraphicsManager.h>

/// @todo íäè€âªÇ∑ÇÈ
#include <core/graphics/Direct3D11/Direct3D11.h>
#include <core/graphics/Direct3D11/Direct3D11Sprite.h>
#include <core/graphics/Direct3D11/Direct3D11Fader.h>
#include <core/graphics/DirectWrite/DirectWrite.h>

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
	, bgm_( 0 )
	, sound_( 0 )
	, is_skipped_( false )
{
	get_sound_manager()->stop_all();

	load_story_text_file( file_name );

	/// @todo êÆóù
	if ( get_direct_3d()->getFont() )
	{
		text_y_target_ = static_cast< float >( -get_direct_3d()->getFont()->get_text_height( text_.c_str(), static_cast< float >( get_width() ), static_cast< float >( get_height() ) ) );
	}
	
	sprite_texture_ = get_graphics_manager()->load_texture( "sprite", "media/image/title.png" );

	if ( bg_sprite_layer_list_.empty() )
	{
		bg_sprite_layer_list_.push_back( new BgSpriteLayer( "bg", get_graphics_manager()->load_texture( "bg", "media/image/story-bg-default.jpg" ) ) );
		// bg_sprite_layer_list_.back()->set_size_from_texture();
	}

	get_direct_3d()->getFader()->full_out();
}

StoryTextScene::~StoryTextScene()
{
	get_graphics_manager()->unload_texture( "sprite" );

	for ( auto i = bg_sprite_layer_list_.begin(); i != bg_sprite_layer_list_.end(); ++i )
	{
		get_graphics_manager()->unload_texture( ( *i )->get_name().c_str() );
		delete *i;
	}
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
	BgSpriteLayer* current_layer = 0;

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

		if ( name == "layer" )
		{
			string_t layer_name;
			ss >> layer_name;

			current_layer = new BgSpriteLayer( layer_name.c_str(), get_graphics_manager()->load_texture( layer_name.c_str(), ( std::string( "media/image/" + layer_name ) ).c_str() ) );

			ss >> current_layer->get_src_rect().left() >> current_layer->get_src_rect().top();
			ss >> current_layer->get_src_rect().right() >> current_layer->get_src_rect().bottom();

			bg_sprite_layer_list_.push_back( current_layer );
		}
		else if ( name == "layer-translation" )
		{
			ss >> current_layer->get_translation().value().x() >> current_layer->get_translation().value().y();
			ss >> current_layer->get_translation().target_value().x() >> current_layer->get_translation().target_value().y();
			ss >> current_layer->get_translation().speed().x() >> current_layer->get_translation().speed().y();
		}
		else if ( name == "layer-rotation" )
		{
			ss >> current_layer->get_rotation().value() >> current_layer->get_rotation().target_value() >> current_layer->get_rotation().speed();
		}
		else if ( name == "layer-scale" )
		{
			ss >> current_layer->get_scale().value() >> current_layer->get_scale().target_value() >> current_layer->get_scale().speed();
		}
		else if ( name == "layer-color" )
		{
			ss >> current_layer->get_color().value().r() >> current_layer->get_color().value().g() >> current_layer->get_color().value().b() >> current_layer->get_color().value().a();
			ss >> current_layer->get_color().target_value().r() >> current_layer->get_color().target_value().g() >> current_layer->get_color().target_value().b() >> current_layer->get_color().value().a();
			ss >> current_layer->get_color().speed().r() >> current_layer->get_color().speed().g() >> current_layer->get_color().speed().b() >> current_layer->get_color().speed().a();
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
 * ÉÅÉCÉìÉãÅ[Évèàóù
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
			sound_->fade_out( Sound::VOLUME_FADE_SPEED_FAST );

			if ( bgm_ )
			{
				bgm_->fade_out();
			}

			get_direct_3d()->getFader()->fade_out();
		}

		if ( sound_->is_fade_full_out() && ( ! bgm_ || bgm_->is_fade_full_out() ) && get_direct_3d()->getFader()->is_full_out() )
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

	for ( auto i = bg_sprite_layer_list_.begin(); i != bg_sprite_layer_list_.end(); ++i )
	{
		( *i )->update();
	}
}

/**
 * ï`âÊ
 */
void StoryTextScene::render()
{
	get_direct_3d()->clear_default_view();
	get_direct_3d()->getSprite()->begin();

	{
		render_technique( "|sprite", [this]
		{
			for ( auto j = bg_sprite_layer_list_.begin(); j != bg_sprite_layer_list_.end(); ++j )
			{

				Matrix r, s, t;

				r.set_rotation_z( math::degree_to_radian( ( *j )->get_rotation().value() ) );
				s.set_scaling( ( *j )->get_scale().value(), ( *j )->get_scale().value(), 1.f );
				t.set_translation( ( *j )->get_translation().value().x(), ( *j )->get_translation().value().y(), 0.f );

				get_direct_3d()->getSprite()->set_transform( r * s * t );
				get_direct_3d()->getSprite()->draw( ( *j )->get_texture(), ( *j )->get_src_rect(), ( *j )->get_color().value() );
			}
		} );
	}

	get_direct_3d()->getSprite()->end();

	// text
	/// @todo êÆóù
	if ( get_direct_3d()->getFont() )
	{
		get_direct_3d()->begin2D();
		get_direct_3d()->getFont()->begin();
		get_direct_3d()->getFont()->draw_text_center( -1.f, text_y_ - 1.f, static_cast< float >( get_width() ), static_cast< float >( get_height() ), text_.c_str(), text_border_color_ );
		get_direct_3d()->getFont()->draw_text_center( +1.f, text_y_ - 1.f, static_cast< float >( get_width() ), static_cast< float >( get_height() ), text_.c_str(), text_border_color_ );
		get_direct_3d()->getFont()->draw_text_center( -1.f, text_y_ + 1.f, static_cast< float >( get_width() ), static_cast< float >( get_height() ), text_.c_str(), text_border_color_ );
		get_direct_3d()->getFont()->draw_text_center( +1.f, text_y_ + 1.f, static_cast< float >( get_width() ), static_cast< float >( get_height() ), text_.c_str(), text_border_color_ );
		get_direct_3d()->getFont()->draw_text_center( 0.f, text_y_, static_cast< float >( get_width() ), static_cast< float >( get_height() ), text_.c_str(), text_color_ );
		get_direct_3d()->getFont()->end();
		get_direct_3d()->end2D();
	}

	get_direct_3d()->begin3D();
	get_direct_3d()->renderText();

	render_fader();

	// loading ...
	if ( is_skipped_ )
	{
		get_direct_3d()->getSprite()->begin();

		render_technique( "|sprite", [this]
		{
			win::Rect src_rect = win::Rect::Size( 512, 0, 272, 128 );
			win::Point dst_point( get_width() - src_rect.width(), get_height() - src_rect.height() );

			get_direct_3d()->getSprite()->draw( dst_point, sprite_texture_, src_rect );
		} );

		get_direct_3d()->getSprite()->end();
	}

	get_direct_3d()->end3D();
}

} // namespace blue_sky