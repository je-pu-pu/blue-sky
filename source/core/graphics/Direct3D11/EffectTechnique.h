#pragma once

#include <core/graphics/EffectTechnique.h>

struct ID3DX11EffectTechnique;

namespace core::graphics::direct_3d_11
{
	class Direct3D11;

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

	/**
	 * 中身（D3DX の technique とパス）を差し替える
	 *
	 * シェーダーを読み直したときに、このオブジェクト自体は残したまま中身だけを入れ替える。
	 * 描画側が持っているこのオブジェクトへのポインタは、読み直しの後もそのまま使える。
	 * nullptr を渡すと、パスが無い（何も描かない）状態になる（読み直したシェーダーから technique が消えた場合）。
	 */
	void rebind( Direct3D*, ID3DX11EffectTechnique* );
	
	// PassList& get_pass_list() { return pass_list_; }
	const PassList& get_pass_list() const override { return pass_list_; }

}; // class EffectTechinique

} // namespace core::graphics::direct_3d_11