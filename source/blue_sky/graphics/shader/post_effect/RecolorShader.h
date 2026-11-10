#pragma once

#include "../Shader.h"

namespace blue_sky::graphics::shader::post_effect
{

/**
 * 色保持リカラー用ポストエフェクトシェーダー（オフライン recolor_lab の HLSL 版）
 *
 * テクスチャ: t0 = ニューラル出力, t1 = 元シーン（色の基準）
 * Lab 空間で L=暗化制限・a 控えめ・b 青方向増幅/黄カット。
 */
class RecolorShader : public Shader< RecolorShader, 0 >
{
public:
	static inline const ParameterInfoList parameter_info_list = {
		{ ParameterType::FLOAT, "blue",      3.0f,  0.f, 8.f   }, // 青方向(db<0)の増幅
		{ ParameterType::FLOAT, "yellow",    0.0f,  0.f, 2.f   }, // 黄方向(db>0)の倍率
		{ ParameterType::FLOAT, "a_spread",  0.5f,  0.f, 1.f   }, // a 軸のスプレッド
		{ ParameterType::FLOAT, "dark_clip", 60.0f, 0.f, 100.f }, // 暗化の許容(L 単位)
	};

public:
	RecolorShader( GraphicsManager* graphics_manager, const char_t* input_layout_name = "main", const char_t* effect_technique_name = "post_effect_recolor" )
		: Shader( graphics_manager, input_layout_name, effect_technique_name, { ShaderStage::PS } )
	{

	}

	RecolorShader* clone() const override { return new RecolorShader( *this ); }
};

} // namespace blue_sky::graphics::shader::post_effect
