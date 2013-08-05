#include "EndingScene.h"
#include "GraphicsManager.h"
#include "DrawingModel.h"
#include "DrawingLine.h"
#include "DrawingModelManager.h"
#include "Sound.h"
#include "SoundManager.h"
#include "GameMain.h"

#include "Direct3D11.h"
#include "Direct3D11Fader.h"
#include "Direct3D11Color.h"
#include "Direct3D11Sprite.h"
#include "Direct3D11Effect.h"
#include "Direct3D11TextureManager.h"

#include <game/Texture.h>
#include <game/ElapsedTimer.h>

#include <win/Rect.h>

#include <common/exception.h>

#include <fstream>

#include "memory.h"

namespace blue_sky
{

EndingScene::EndingScene( const GameMain* game_main )
	: Scene( game_main )
	, elapsed_timer_( new game::ElapsedTimer( get_main_loop() ) )
{
	get_graphics_manager()->setup_loader();
	
	load_sequence_file();
	current_drawing_model_index_ = 0;
	current_drawing_model_ = drawing_model_list_.front();
	drawing_model_elapsed_time_ = 0.f;
	drawing_model_stop_time_ = 0.f;
	drawing_model_offset_ = 0.f;

	get_graphics_manager()->cleanup_loader();

	bgm_ = get_sound_manager()->load_music( "opening-of-the-day" );

	bg_texture_ = get_direct_3d()->getTextureManager()->load( "bg", "media/texture/cloth.png" );

	get_direct_3d()->getFader()->full_in();

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

		DrawingModel* model = get_drawing_model_manager()->load( model_file_name.c_str() );

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
 * ƒƒCƒ“ƒ‹[ƒvˆ—
 *
 */
void EndingScene::update()
{
	elapsed_timer_->update();
	drawing_model_elapsed_time_ += elapsed_timer_->get_elapsed_sec();

	bool is_last = current_drawing_model_index_ == drawing_model_list_.size() - 1;

	if ( get_visible_drawing_line_part_count() > current_drawing_model_->get_line()->get_part_count() )
	{
		drawing_model_stop_time_ += elapsed_timer_->get_elapsed_sec();

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
}

/**
 * •`‰æ
 *
 */
void EndingScene::render()
{ 
	update_constant_buffer_for_sprite_frame();
	
	get_direct_3d()->clear( Color::from_256( 0xFF, 0xAA, 0x11 ) );

	render_bg();

	render_drawing_line();
}

void EndingScene::render_bg()
{
	get_direct_3d()->getSprite()->begin();

	Direct3D::EffectTechnique* technique = get_direct_3d()->getEffect()->getTechnique( "|sprite" );

	for ( Direct3D::EffectTechnique::PassList::const_iterator i = technique->getPassList().begin(); i != technique->getPassList().end(); ++i )
	{
		( *i )->apply();

		win::Rect dst_rect( 0, 0, get_width(), get_height() );

		get_direct_3d()->getSprite()->draw( dst_rect, bg_texture_ );
	}

	get_direct_3d()->getSprite()->end();
}

void EndingScene::render_drawing_line()
{
	get_direct_3d()->setInputLayout( "line" );

	auto technique = get_direct_3d()->getEffect()->getTechnique( "|drawing_line" );

	for ( auto i = technique->getPassList().begin(); i != technique->getPassList().end(); ++i )
	{
		( *i )->apply();

		/// @todo ®—‚·‚é
		{
			ObjectConstantBufferData object_constant_buffer_data;

			object_constant_buffer_data.world = XMMatrixTranspose( XMMatrixTranslation( 0.f, drawing_model_offset_, 0.f ) );
			object_constant_buffer_data.color = Color( 0.f, 0.f, 0.f, 0.f );

			get_game_main()->get_object_constant_buffer()->update( & object_constant_buffer_data );
		}

		get_game_main()->get_game_constant_buffer()->bind_to_all(); /// !!!
		get_game_main()->get_frame_constant_buffer()->bind_to_all(); /// !!!
		get_game_main()->get_frame_drawing_constant_buffer()->bind_to_all(); /// !!!
		get_game_main()->get_object_constant_buffer()->bind_to_all(); /// !!!

		current_drawing_model_->get_line()->render_part( get_visible_drawing_line_part_count() );
	}

	render_fader();
}

int EndingScene::get_visible_drawing_line_part_count() const
{
	return static_cast< int >( drawing_model_elapsed_time_ * 8.f );
}

} // namespace blue_sky