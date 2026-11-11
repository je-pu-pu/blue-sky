#pragma once

#include <core/graphics/EffectTechnique.h>

#include <vector>

namespace core::graphics::direct_3d_11
{
	class Direct3D11;
	struct EffectPassSetup;

/**
 * effect の technique（パスの一覧）
 *
 */
class EffectTechnique : public core::graphics::EffectTechnique

{
public:
	using Direct3D = Direct3D11;
	using PassSetupList = std::vector< EffectPassSetup >;

private:
	PassList			pass_list_;

public:
	EffectTechnique( Direct3D*, const PassSetupList* );
	~EffectTechnique();

	/**
	 * 中身（パス）を差し替える
	 *
	 * シェーダーを読み直したときに、このオブジェクト自体は残したまま中身だけを入れ替える。
	 * 描画側が持っているこのオブジェクトへのポインタは、読み直しの後もそのまま使える。
	 * nullptr を渡すと、パスが無い（何も描かない）状態になる（読み直したシェーダーから technique が消えた場合）。
	 */
	void rebind( Direct3D*, const PassSetupList* );

	const PassList& get_pass_list() const override { return pass_list_; }

}; // class EffectTechinique

} // namespace core::graphics::direct_3d_11
