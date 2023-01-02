#pragma once

#include "../Shader.h"

namespace blue_sky::graphics::shader::post_effect
{

/**
 * �菑�����ϊ��|�X�g�G�t�F�N�g�p�V�F�[�_�[
 *
 */
class HandDrawingShader : public Shader< HandDrawingShader, 0 >
{
public:
	static inline const ParameterInfoList parameter_info_list = {
		{ ParameterType::FLOAT, "uv_factor",   20.f,         0.f,      100.f }, // UV �W��
		{ ParameterType::FLOAT, "time_factor", 10.f,         0.f,      100.f }, // ���ԌW��
		{ ParameterType::FLOAT, "gain1",       0.001f,       0.00001f, 0.01f }, // �U�� 1
		{ ParameterType::FLOAT, "gain2",       0.0005f,      0.00001f, 0.01f }, // �U�� 2
		{ ParameterType::FLOAT, "gain3",       0.00025f,     0.00001f, 0.01f }, // �U�� 3
		{ ParameterType::COLOR, "color",       std::nullopt, 0.00001f, 0.01f }, // test
	};

public:
	HandDrawingShader( const char_t* input_layout_name = "main", const char_t* effect_technique_name = "post_effect_hand_drawing" )
		: Shader( input_layout_name, effect_technique_name, { ShaderStage::PS } )
	{

	}

	HandDrawingShader* clone() const override { return new HandDrawingShader( *this ); }

	void bind() const override
	{
		Shader::bind();

		get_frame_constant_buffer()->bind_to_ps();
	}
};

} // namespace blue_sky::graphics::shader::post_effect
