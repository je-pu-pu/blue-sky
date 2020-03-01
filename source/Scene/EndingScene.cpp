#include "EndingScene.h"
#include "GameMain.h"
#include "Input.h"

#include <blue_sky/graphics/GraphicsManager.h>
#include <blue_sky/graphics/Model.h>
#include <blue_sky/graphics/Line.h>

/// @todo ���ۉ�����
#include <blue_sky/graphics/Fader.h>
#include <core/graphics/Direct3D11/Sprite.h>

#include <core/sound/SoundManager.h>
#include <core/sound/Sound.h>

#include <game/ElapsedTimer.h>

#include <win/Rect.h>

#include <common/exception.h>

#include <fstream>

namespace blue_sky
{

EndingScene::EndingScene( const GameMain* game_main )
	: Scene( game_main )
	, elapsed_timer_( new game::ElapsedTimer( get_main_loop() ) )
	, line_type_( 0 )
	, drawing_accent_scale_( 1.f )
	, in_fade_out_( false )
{
	load_sequence_file();
	current_drawing_model_index_ = 0;
	current_drawing_model_ = drawing_model_list_[ current_drawing_model_index_ ];
	drawing_model_elapsed_time_ = 0.f;
	drawing_model_stop_time_ = 0.f;
	drawing_model_offset_ = 0.f;

	bgm_ = get_sound_manager()->load_music( "opening-of-the-day" );
	switch_sound_ = get_sound_manager()->load( "switch-on" );
	click_sound_ = get_sound_manager()->load( "click" );

	bg_texture_ = get_graphics_manager()->load_named_texture( "bg", "media/texture/cloth.png" );

	get_graphics_manager()->get_fader()->full_in();

	elapsed_timer_->reset();
	bgm_->play( false );
}

EndingScene::~EndingScene()
{
	
}

void EndingScene::load_sequence_file()
{
	std::ifstream in( "media/stage/ending-drawing-model.list" );
	
	if ( ! in.good() )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( string_t( "load sequence file failed." ) );
	}

	while ( in.good() )
	{
		string_t model_file_name;
		in >> model_file_name;

		if ( model_file_name.empty() )
		{
			continue;
		}

		Model* model = get_graphics_manager()->load_model( model_file_name.c_str() );

		if ( ! model || ! model->get_line() )
		{
			COMMON_THROW_EXCEPTION_MESSAGE( string_t( "load model file failed. ( " ) + model_file_name + " )" );
		}

		drawing_model_list_.push_back( model );
	}

	if ( drawing_model_list_.empty() )
	{
		COMMON_THROW_EXCEPTION_MESSAGE( "list is empty." );
	}
}

/**
 * ���C�����[�v����
 *
 */
void EndingScene::update()
{
	Scene::update();

	elapsed_timer_->update();
	drawing_model_elapsed_time_ += elapsed_timer_->get_elapsed_sec();

	bool is_last = current_drawing_model_index_ == drawing_model_list_.size() - 1;
	bool is_end = false;

	if ( get_visible_drawing_line_part_count() > current_drawing_model_->get_line()->get_part_count() )
	{
		drawing_model_stop_time_ += elapsed_timer_->get_elapsed_sec();
		
		if ( is_last )
		{
			is_end = true;
		}

		if ( drawing_model_stop_time_ >= 2.f && ! is_last )
		{
			drawing_model_offset_ += elapsed_timer_->get_elapsed_sec() * 0.5f;

			if ( drawing_model_offset_ >= 2.f )
			{
				drawing_model_elapsed_time_ = 0.f;
				drawing_model_stop_time_ = 0.f;
				drawing_model_offset_ = 0.f;
				
				current_drawing_model_index_++;
				current_drawing_model_ = drawing_model_list_[ current_drawing_model_index_ ];
			}
		}
	}

	if ( get_input()->push( Input::A ) )
	{
		if ( is_end )
		{
			in_fade_out_ = true;
		}
		else
		{
			line_type_++;

			if ( line_type_ >= Line::LINE_TYPE_MAX )
			{
				line_type_ = 0;
			}

			switch_sound_->play( false );
		}
	}
	
	float last_drawing_accent_scale_ = drawing_accent_scale_;

	int wheel = get_input()->pop_mouse_wheel_queue();

	if ( wheel > 0 )
	{
		if ( drawing_accent_scale_ == 0.f ) drawing_accent_scale_ = 1.f;
		else if ( drawing_accent_scale_ == 1.f ) drawing_accent_scale_ = 10.f;
	}
	else if ( wheel < 0 )
	{
		if ( drawing_accent_scale_ == 1.f ) drawing_accent_scale_ = 0.f;
		else if ( drawing_accent_scale_ == 10.f ) drawing_accent_scale_ = 1.f;
	}

	if ( drawing_accent_scale_ != last_drawing_accent_scale_ )
	{
		click_sound_->play( false );
	}

	if ( in_fade_out_ )
	{
		if ( get_graphics_manager()->get_fader()->fade_out() )
		{
			set_next_scene( "title" );
		}
	}
}

/**
 * �`��
 *
 */
void EndingScene::render()
{ 
	update_constant_buffer_for_sprite_frame( line_type_, drawing_accent_scale_ );
	
	get_direct_3d()->set_default_render_target();
	get_direct_3d()->set_default_viewport();

	get_direct_3d()->clear_default_view( Color::from_256( 0xFF, 0xAA, 0x11 ) );

	render_bg();

	render_drawing_line();
}

void EndingScene::render_bg()
{
	get_direct_3d()->get_sprite()->begin();

	render_technique( "|sprite", [this]
	{
		win::Rect dst_rect( 0, 0, get_width(), get_height() );

		get_direct_3d()->get_sprite()->draw( dst_rect, bg_texture_ );
	} );

	get_direct_3d()->get_sprite()->end();
}

void EndingScene::render_drawing_line()
{
	get_graphics_manager()->set_input_layout( "line" );

	render_technique( "|drawing_line", [this]
	{
		/// @todo ��������
		{
			ObjectConstantBufferData object_constant_buffer_data;

			object_constant_buffer_data.world = ( Matrix().set_scaling( 1.25f, 1.25f, 1.f ) * Matrix().set_translation( 0.f, drawing_model_offset_, 0.f ) ).transpose();
			object_constant_buffer_data.color = Color( 0.f, 0.f, 0.f, 0.f );

			get_graphics_manager()->get_shared_object_render_data()->update( & object_constant_buffer_data );
		}

		bind_all_render_data();

		current_drawing_model_->get_line()->render_part( get_visible_drawing_line_part_count() );
	} );

	render_fader();
}

int EndingScene::get_visible_drawing_line_part_count() const
{
	return static_cast< int >( drawing_model_elapsed_time_ * 12.f );
}

} // namespace blue_sky