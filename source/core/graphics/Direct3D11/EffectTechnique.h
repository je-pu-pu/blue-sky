#pragma once

#include <core/graphics/EffectTechnique.h>

class Direct3D11;

struct ID3DX11EffectTechnique;

namespace core::graphics::direct_3d_11
{

/**
 * Direct3D 11 Effect Technique のラッパークラス
 *
 */
class EffectTechnique : public core::graphics::EffectTechnique

{
public:
	using Direct3D = Direct3D11;

private:
	PassList			pass_list_;

public:
	EffectTechnique( Direct3D*, ID3DX11EffectTechnique* );
	~EffectTechnique();
	
	// PassList& get_pass_list() { return pass_list_; }
	const PassList& get_pass_list() const override { return pass_list_; }

}; // class EffectTechinique

} // namespace core::graphics::direct_3d_11