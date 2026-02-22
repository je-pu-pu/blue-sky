#include "StoryTextScene.h"

#include <blue_sky/GameMain.h>
#include <blue_sky/Input.h>

#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/graphics/Fader.h>

#include <core/sound/SoundManager.h>
#include <core/sound/Sound.h>

#include <core/graphics/Sprite.h>
#include <core/graphics/BgSpriteLayer.h>

#include <win/Rect.h>

#include <common/string.h>

#include <fstream>
#include <sstream>

namespace blue_sky
{

StoryTextScene::StoryTextScene( const char* file_name, const char* next_scene_name )
	: text_y_( static_cast< float >( get_height() ) )
	, text_y_target_( 0.f )
	, text_scroll_speed_( 0.5f )
	, text_color_( Color::from_256( 255, 255, 255, 127 ) )
	, text_border_color_( Color::from_256( 0, 0, 0, 127 ) )
	, next_scene_name_( next_scene_name )
	, sprite_texture_( 0 )
	, bgm_( 0 )
	, sound_( 0 )
	, is_skipped_( false )
{
	get_sound_manager()->stop_all();

	load_story_text_file( file_name );

	text_y_target_ = -get_graphics_manager()->get_text_height( text_.c_str(), static_cast< float >( get_width() ), static_cast< float >( get_height() ) );
	
	sprite_texture_ = get_graphics_manager()->load_named_texture( "sprite", "media/image/title.png" );

	if ( bg_sprite_layer_list_.empty() )
	{
		bg_sprite_layer_list_.push_back( std::make_unique< BgSpriteLayer >( "bg", get_graphics_manager()->load_named_texture( "bg", "media/image/story-bg-default.jpg" ) ) );
	}

	get_graphics_manager()->get_fader()->full_out();
}

StoryTextScene::~StoryTextScene() = default;

void StoryTextScene::load_story_text_file( const char* file_name )
{
	std::ifstream in( file_name );
	
	if ( ! in.good() )
	{
		set_next_scene( next_scene_name_ );
		return;
	}

	bool in_text = false;
	BgSpriteLayer* current_layer = nullptr;

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
		std::string command;

		ss << line;
		ss >> command;

		if ( command == "layer" )
		{
			string_t layer_name;
			ss >> layer_name;

			auto layer = std::make_unique< BgSpriteLayer >( layer_name.c_str(), get_graphics_manager()->load_named_texture( layer_name.c_str(), ( std::string( "media/image/" + layer_name ) ).c_str() ) );
			current_layer = layer.get();

			ss >> current_layer->get_src_rect().left() >> current_layer->get_src_rect().top();
			ss >> current_layer->get_src_rect().right() >> current_layer->get_src_rect().bottom();

			bg_sprite_layer_list_.push_back( std::move( layer ) );
		}
		else if ( command == "layer-translation" )
		{
			ss >> current_layer->get_translation().value().x() >> current_layer->get_translation().value().y();
			ss >> current_layer->get_translation().target_value().x() >> current_layer->get_translation().target_value().y();
			ss >> current_layer->get_translation().speed().x() >> current_layer->get_translation().speed().y();
		}
		else if ( command == "layer-rotation" )
		{
			ss >> current_layer->get_rotation().value() >> current_layer->get_rotation().target_value() >> current_layer->get_rotation().speed();
		}
		else if ( command == "layer-scale" )
		{
			ss >> current_layer->get_scale().value() >> current_layer->get_scale().target_value() >> current_layer->get_scale().speed();
		}
		else if ( command == "layer-color" )
		{
			ss >> current_layer->get_color().value().r() >> current_layer->get_color().value().g() >> current_layer->get_color().value().b() >> current_layer->get_color().value().a();
			ss >> current_layer->get_color().target_value().r() >> current_layer->get_color().target_value().g() >> current_layer->get_color().target_value().b() >> current_layer->get_color().value().a();
			ss >> current_layer->get_color().speed().r() >> current_layer->get_color().speed().g() >> current_layer->get_color().speed().b() >> current_layer->get_color().speed().a();
		}
		else if ( command == "bgm" )
		{
			std::string bgm_name;
			float_t volume = 1.f;

			ss >> bgm_name >> volume;

			bgm_ = get_sound_manager()->load_music( "bgm", bgm_name.c_str() );
			bgm_->set_volume( volume );
			bgm_->play( false );
		}
		else if ( command == "text-color" )
		{
			uint_t hex;

			ss >> std::hex >> hex;

			text_color_ = Color::from_hex( hex );
		}
		else if( command == "text-border-color" )
		{
			uint_t hex;

			ss >> std::hex >> hex;

			text_border_color_ = Color::from_hex( hex );
		}
		else if ( command == "text" )
		{
			in_text = true;
		}
		else if ( command == "sound" )
		{
			std::string sound_name;
			ss >> sound_name;
			sound_ = get_sound_manager()->load( "sound", sound_name.c_str() );
			sound_->play( false );
		}
		else if ( command == "next-scene" )
		{
			ss >> next_scene_name_;
		}
		else if ( command == "next-stage-name" )
		{
			ss >> next_stage_name_;
		}
	}
}

/**
 * メインループ処理
 *
 */
void StoryTextScene::update()
{
	float speed = text_scroll_speed_;

	if ( ! text_.empty() )
	{
		if ( get_input()->press( Input::Button::A ) )
		{
			speed *= 8.f;
		}
		if ( get_input()->press( Input::Button::B ) )
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
			if ( get_input()->push( Input::Button::A ) || ! sound_->is_playing() )
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

			get_graphics_manager()->get_fader()->fade_out();
		}

		if ( sound_->is_fade_full_out() && ( ! bgm_ || bgm_->is_fade_full_out() ) && get_graphics_manager()->get_fader()->is_full_out() )
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
		get_graphics_manager()->get_fader()->fade_in();
	}

	for ( auto i = bg_sprite_layer_list_.begin(); i != bg_sprite_layer_list_.end(); ++i )
	{
		( *i )->update();
	}
}

/**
 * 描画
 */
void StoryTextScene::render()
{
	get_graphics_manager()->clear_default_view();

	auto* sprite = get_graphics_manager()->get_sprite();
	sprite->begin();

	{
		render_technique( "|sprite", [this, sprite]
		{
			for ( auto j = bg_sprite_layer_list_.begin(); j != bg_sprite_layer_list_.end(); ++j )
			{

				Matrix r, s, t;

				r.set_rotation_z( math::degree_to_radian( ( *j )->get_rotation().value() ) );
				s.set_scaling( ( *j )->get_scale().value(), ( *j )->get_scale().value(), 1.f );
				t.set_translation( ( *j )->get_translation().value().x(), ( *j )->get_translation().value().y(), 0.f );

				sprite->set_transform( r * s * t );
				sprite->draw( ( *j )->get_texture(), ( *j )->get_src_rect(), ( *j )->get_color().value() );
			}
		} );
	}

	sprite->end();

	// text
	{
		get_graphics_manager()->set_default_render_target( false );
		get_graphics_manager()->set_default_viewport();

		get_graphics_manager()->draw_text_center( 0.f, text_y_, static_cast< float >( get_width() ), static_cast< float >( get_height() ), text_.c_str(), core::graphics::TextStyle{ text_color_, text_border_color_, 2.f } );
	}

	render_fader();

	// loading ...
	if ( is_skipped_ )
	{
		sprite->begin();

		render_technique( "|sprite", [this, sprite]
		{
			win::Rect src_rect = win::Rect::Size( 512, 0, 272, 128 );
			win::Point dst_point( get_width() - src_rect.width(), get_height() - src_rect.height() );

			sprite->draw( dst_point, sprite_texture_, src_rect );
		} );

		sprite->end();
	}
}

} // namespace blue_sky