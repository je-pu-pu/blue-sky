#pragma once

#include <unordered_map>

class Direct3D11;
class Direct3D11EffectPass;

struct ID3DX11Effect;

namespace core::graphics::direct_3d_11
{

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

	using TechniqueList		= std::unordered_map< const char*, Technique* >;

private:
	Direct3D*			direct_3d_;
	ID3DX11Effect*		effect_;

	TechniqueList		technique_list_;

public:
	explicit Effect( Direct3D* );
	~Effect();
	
	void load( const char* );

	Technique* get_technique( const char* name );

}; // class Effect

} // namespace namespace core::graphics::direct_3d_11
