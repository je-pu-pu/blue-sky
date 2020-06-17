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
public:
	struct ConstantBufferData : public BaseConstantBufferData< 0 >
	{
		float_t UvFactor	= 20.f;		// UV 係数
		float_t TimeFactor	= 10.f;		// 時間係数
		float_t Gain1		= 0.001f;	// 振幅 1
		float_t Gain2		= 0.0005f;	// 振幅 2
		float_t Gain3		= 0.00025f;	// 振幅 3
		float_t dummy[ 3 ]	= { 0.f };
	};

	using ConstantBuffer = blue_sky::ConstantBufferWithData< ConstantBufferData >;

private:
	ConstantBuffer constant_buffer_;

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

		constant_buffer_.update();
		constant_buffer_.bind_to_all();

		get_texture_at( 0 )->bind_to_ps( 0 );
	}

	float_t& getUvFactor() { return constant_buffer_.data().UvFactor; }
	float_t& getTimeFactor() { return constant_buffer_.data().TimeFactor; }
	float_t& getGain1() { return constant_buffer_.data().Gain1; }
	float_t& getGain2() { return constant_buffer_.data().Gain2; }
	float_t& getGain3() { return constant_buffer_.data().Gain3; }
};


} // namespace blue_sky::graphics::shader::post_effect
