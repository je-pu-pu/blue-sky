#pragma once

#include <memory>
#include <string>
#include <unordered_map>

struct ID3DX11Effect;

namespace core::graphics::direct_3d_11
{
	class Direct3D11;
	class EffectTechnique;
	class EffectPass;

/**
 * Direct3D 11 Effect のラッパークラス
 *
 */
class Effect
{
public:
	using Direct3D			= Direct3D11;

	using Technique			= EffectTechnique;
	using Pass				= EffectPass;

	/// technique の名前 → technique（キーは文字列。以前はポインタの値をキーにしていて、同じ名前でも別のエントリになっていた）
	using TechniqueList		= std::unordered_map< std::string, std::unique_ptr< Technique > >;

private:
	Direct3D*			direct_3d_;
	ID3DX11Effect*		effect_;

	TechniqueList		technique_list_;

public:
	explicit Effect( Direct3D* );
	~Effect();
	
	/**
	 * エフェクトファイルを読む（読み直しにも使う）
	 *
	 * コンパイルに成功してから差し替える。失敗したら例外を投げ、それまでのエフェクトをそのまま使い続ける。
	 * 取得済みの technique のオブジェクトは残し、中身だけを新しいエフェクトのものに入れ替えるので、
	 * 描画側が持っているポインタは読み直しの後も有効。
	 */
	void load( const char* );
	void clear();

	/// technique を名前で引く。無ければ nullptr
	Technique* get_technique( const char* name );

}; // class Effect

} // namespace namespace core::graphics::direct_3d_11
