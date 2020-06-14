#pragma once

#include "DefaultShader.h"

namespace blue_sky::graphics::shader::post_effect
{

/**
 * 手書き風変換ポストエフェクト用シェーダー
 *
 */
class HandDrawingShader : public DefaultShader
{
private:

public:
	HandDrawingShader( const char_t* input_layout_name = "main", const char_t* effect_technique_name = "post_effect_hand_drawing" )
		: DefaultShader( input_layout_name, effect_technique_name )
	{

	}

	HandDrawingShader* clone() const override { return new HandDrawingShader( *this ); }

	void bind() const override
	{
		/// @todo 必要なものだけをバインドするようにする
		get_game_constant_buffer()->bind_to_all();
		get_frame_constant_buffer()->bind_to_all();
		get_frame_drawing_constant_buffer()->bind_to_all();
		
		get_object_constant_buffer()->bind_to_vs();

		get_texture_at( 0 )->bind_to_ps( 0 );
	}
};


} // namespace blue_sky::graphics::shader::post_effect
